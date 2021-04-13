#include "socket_communication.h"

client_data::client_data(int s, sockaddr_in in, socklen_t len)
{
    socket = s;
    cli_address = in;
    clilen = len;
    is_authorized = false;
    is_active = true;
    return;
}

socket_communication::socket_communication()
{
    sockfd = -1;
    bzero(&srv_address, sizeof(srv_address));
    srv_address.sin_family = AF_INET;
    srv_address.sin_addr.s_addr = INADDR_ANY;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    stop_server = false;
    loop = nullptr;
    id_count = 0;
    return;
}

void socket_communication::set_evloop_address(ev_loop *a)
{
    loop = a;
    return;
}

void socket_communication::start(int port)
{
    port_number = port;
    srv_address.sin_port = htons(port_number);
    if (bind(sockfd, (struct sockaddr *) &srv_address, sizeof(srv_address)) < 0)
    {
        std::cout << "Unable to bind at port " << port_number << std::endl;
        return;
    }
    std::cout << "SERVER: sucsesfully binded at port " << port_number << "\n";
    incoming_connections_listener();
}

void socket_communication::incoming_connections_listener()
{
    while (!stop_server)
    {
        listen(sockfd, 10);
        socklen_t clilen;
        sockaddr_in cli_addr;
        bzero(&cli_addr, sizeof(cli_addr));
        bzero(&clilen, sizeof(clilen));
        int newsock = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsock <= 0)
            continue;
        client_data new_client(newsock, cli_addr, clilen);
        std::thread *thr = new std::thread([this, new_client] () { client_listener(new_client); });
        std::lock_guard<std::mutex> lock(threads_list_lock);
        threads_list.push_back(thr);
    }
}

void socket_communication::client_listener(client_data curr_client)
{
    while (!stop_server && curr_client.is_active)
    {
        char buffer[MAX];
        bzero(buffer, MAX);
        int s = recv(curr_client.socket, buffer, MAX, 0);
        if (s == 0)
            continue;
        std::wstring request = unicode_to_wstring(buffer, s);
        parser(request, &curr_client);
    }
    //exit
    std::lock_guard<std::mutex> lock(active_client_lock);
    active_client.erase(curr_client.nickname);
    return;
}

void socket_communication::send_to_client(std::wstring request)
{
    std::lock_guard<std::mutex> lock(active_client_lock);
    std::wcout << request << std::endl;
    for (auto x = active_client.begin(); x != active_client.end(); x++)
    {
        if (x->second->is_authorized)
            send(x->second->socket, unicode_get_bytes(request), request.size()*2, 0);
    }
}

//request example : TYPE=<global>;SENDER=<x1larus>;RECIEVERS=<all>;MSG=<some shit>.
void socket_communication::parser(std::wstring request, client_data *client)
{
    std::map<std::string, std::wstring> result;
    if (request.empty())
        return;
    bool is_value = false;
    std::wstring key;
    std::wstring value;
    for (unsigned int i = 0; i < request.size(); i++)
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
    if (!client->is_authorized && command == "login")
    {
        std::wstring login = result["LOGIN"];
        std::wstring password = result["PASSWORD"];
        if (login.empty() || password.empty())
            return;
        client_login(login, password, client);
        return;
    }
    //register maybe?

    //exit
    if (command == "logout")
    {
        client_logout(client->nickname);
        return;
    }
    if (client->is_authorized)
        loop->add_task(command, result);
    return;
}

bool socket_communication::client_login(std::wstring login, std::wstring password, client_data *client)
{
    std::lock_guard<std::mutex> lock(active_client_lock);
    for (auto x : active_client)
    {
        if (x.first == login)
            return false;
    }
    client->is_authorized = true;
    client->nickname = login;
    active_client[login] = client;
    return true;
}

void socket_communication::client_logout(std::wstring login)
{
    std::lock_guard<std::mutex> lock(active_client_lock);
    send(active_client[login]->socket, "exit", 5, 0);
    shutdown(active_client[login]->socket, SHUT_RDWR);
    close(active_client[login]->socket);
    return;
}