#include "socket_communication.h"

client_data::client_data(int s, sockaddr_in in, socklen_t len)
{
    socket = s;
    cli_address = in;
    clilen = len;
    is_authorized = false;
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
    while (!stop_server)
    {
        char buffer[MAX];
        bzero(buffer, MAX);
        int s = recv(curr_client.socket, buffer, MAX, 0);
        if (s == 0)
            continue;
        std::wstring request = unicode_to_wstring(buffer, s);
        std::map<std::string, std::wstring> args;
        args["request"] = request;
        if (!curr_client.is_authorized)
            client_login(&curr_client, args);
        loop->add_task("recv_parse", args);
    }
}

void socket_communication::send_to_client(std::wstring request)
{
    std::lock_guard<std::mutex> lock(active_client_lock);
    std::wcout << request << std::endl;
    for (auto x = active_client.begin(); x != active_client.end(); x++)
    {
        send(x->second.socket, unicode_get_bytes(request), request.size()*2, 0);
    }
}

void socket_communication::client_login(client_data *client, std::map<std::string, std::wstring> args)
{
    std::map<std::string, std::wstring> result;
    std::wstring request = args["request"];
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
    //parser end

    std::wstring nick = result["LOGIN"];
    if (nick.empty())
        return;
    client->nickname = nick;
    std::lock_guard<std::mutex> lock(active_client_lock);
    active_client[id_count++] = *client;
    return;
}