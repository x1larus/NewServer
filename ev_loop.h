#pragma once
#include "encode.h"
#include "socket_communication.h"
#include <mutex>
#include <condition_variable>
#include <map>
#include <queue>
#include <thread>

class socket_communication;

class ev_loop
{
private:
    std::queue <std::pair<std::string, std::map<std::string, std::wstring> > > task_queue;
    std::condition_variable waiter;
    std::mutex task_queue_lock;
    std::thread *loop_thread;
    socket_communication *sender;
    
    //methods
    void loop();
    void recv_parse(std::map<std::string, std::wstring>);
    void send_for_all(std::map<std::string, std::wstring>);
    void build_request(std::map<std::string, std::wstring>);

public:
    ev_loop();
    void set_sender_address(socket_communication*);
    void add_task(std::string, std::map<std::string, std::wstring>);
};