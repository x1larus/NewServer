#include "socket_communication.h"

client::client(int s, sockaddr_in in, socklen_t len)
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
        client new_client(newsock, cli_addr, clilen);
        std::thread *thr = new std::thread([this, new_client] () { client_listener(new_client); });
        std::lock_guard<std::mutex> lock(threads_list_lock);
        threads_list.push_back(thr);
    }
}

void socket_communication::client_listener(client curr_client)
{
    while (!stop_server)
    {
        char buffer[MAX];
        bzero(buffer, MAX);
        int s = recv(curr_client.socket, buffer, MAX, 0);
        if (s == 0)
            continue;
        
        std::wstring newbuf = to_wstring(buffer, s);
        std::wcout << newbuf << std::endl;
        s = send(curr_client.socket, to_char(newbuf), MAX, 0);
    }
}

std::wstring socket_communication::to_wstring(char str[], int size) 
{
    std::wstring result;
    for (int i = 0; i < size; i += 2)
    {
        wchar_t a = str[i] + (str[i+1] * 256);
        result.push_back(a);
    }
    return result;
}

char* socket_communication::to_char(std::wstring str)
{
    char *result = new char[MAX];
    for (u_int32_t i = 0; i < str.size(); i++)
    {
        result[i*2+1] = str[i] / 256;
        result[i*2] = str[i] - (result[i*2+1] * 256);
    }
    return result;
}