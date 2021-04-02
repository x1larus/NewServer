#include "ev_loop.h"

ev_loop::ev_loop()
{
    loop_thread = new std::thread([this] () { loop(); });
}

void ev_loop::set_sender_address(socket_communication *a)
{
    sender = a;
    return;
}

void ev_loop::loop()
{
    while (true)
    {
        std::unique_lock<std::mutex> cv_lock(waiter_lock);
        while (task_queue.empty())
            waiter.wait(cv_lock, [this] () { return !task_queue.empty(); });
        task_queue_lock.lock();
        auto curr_task = task_queue.front();
        task_queue.pop();
        task_queue_lock.unlock();
        std::string command = curr_task.first;
        if (command == "recv_parse")
            recv_parse(curr_task.second);
        if (command == "global")
            build_request(curr_task.second);
    }
}

void ev_loop::add_task(std::string command, std::map<std::string, std::wstring> args)
{
    std::pair<std::string, std::map<std::string, std::wstring> > task(command, args);
    std::lock_guard<std::mutex> lock(task_queue_lock);
    task_queue.push(task);
    waiter.notify_one();
    return;
}

//request example : TYPE=<global>;SENDER=<x1larus>;RECIEVERS=<all>;MSG=<some fucking shit>.

void ev_loop::recv_parse(std::map<std::string, std::wstring> args)
{
    std::map<std::string, std::wstring> result;
    std::wstring request = args["request"];
    if (request.empty())
        return;
    bool is_value = false;
    std::wstring key;
    std::wstring value;
    for (int i = 0; i < request.size(); i++)
    {
        if ((request[i] % 256 == '=' || request[i] % 256 == ';' || request[i] == 0) && !is_value)
            continue;
        if ((request[i] % 256 == '.') && (request[i] / 256 == 0) && !is_value)
            break;
        if ((request[i] % 256 == '<') && (request[i] / 256 == 0) && !is_value) 
        {
            is_value = true;
            continue;
        }
        if ((request[i] % 256 == '>') && (request[i] / 256 == 0) && (request[i+1] % 256 == ';') && (request[i+1] / 256 == 0))
        {
            is_value = false;
            result[unicode_to_ascii(key)] = value;
            key.erase();
            value.erase();
            continue;
        }

        //default
        if (is_value)
            value.push_back(request[i]);
        else
            key.push_back(request[i]);
    }
    
    std::string command = unicode_to_ascii(result["TYPE"]);
    if (command.empty())
        return;
    else add_task(command, result);
    return;
}

void ev_loop::build_request(std::map<std::string, std::wstring> args)
{
    std::wstring request;
    for (auto x = args.begin(); x != args.end(); x++)
    {
        request += ascii_to_wstring(x->first) + ascii_to_wstring("=<") + x->second + ascii_to_wstring(">;");
    }
    request += ascii_to_wstring(".");
    sender->send_to_client(request);
}