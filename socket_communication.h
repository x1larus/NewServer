#pragma once
#include <thread>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <iostream>

constexpr int MAX = 1024;

struct client
{
    int socket;
    //я хз зачем это
    socklen_t clilen;
    sockaddr_in cli_address;
    bool is_authorized;
    client(int, sockaddr_in, socklen_t);
};


class socket_communication
{
private:
    int sockfd;
    sockaddr_in srv_address;
    int port_number;
    std::map<int, client> active_client;   
    std::mutex active_client_lock;
    std::mutex threads_list_lock;
    std::vector<std::thread*> threads_list;
    bool stop_server;   
    
    //methods
    void incoming_connections_listener();
    void client_listener(client);  
    //return TRUE if client added, otherwise, FALSE
    bool add_new_active_client(client);
    std::wstring to_wstring(char[], int);
    char* to_char(std::wstring);

public:
    socket_communication();
    void start(int);

};