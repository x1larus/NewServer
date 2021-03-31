#pragma once
#include "encode.h"
#include "ev_loop.h"
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

constexpr int MAX = 4096;

class ev_loop;

struct client_data
{
    int socket;
    //я хз зачем это
    socklen_t clilen;
    sockaddr_in cli_address;
    bool is_authorized;
    client_data() = default;
    client_data(int, sockaddr_in, socklen_t);
};

class socket_communication
{
private:
    int sockfd;
    sockaddr_in srv_address;
    int port_number;
    std::map<int, client_data> active_client;   
    std::mutex active_client_lock;
    std::mutex threads_list_lock;
    std::vector<std::thread*> threads_list;
    bool stop_server;
    ev_loop *loop;  
    
    //methods
    void incoming_connections_listener();
    void client_listener(client_data);  
    //return TRUE if client added, otherwise, FALSE
    bool add_new_active_client(client_data);

public:
    socket_communication();
    void start(int);
    void set_evloop_address(ev_loop*);
    void send_to_client(std::wstring);
};