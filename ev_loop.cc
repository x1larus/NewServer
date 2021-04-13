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