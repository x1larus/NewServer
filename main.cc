#include "socket_communication.h"
#include "ev_loop.h"
#include <cstdlib>

int main(int argc, char **argv)
{
    socket_communication sender;
    ev_loop loop;
    sender.set_evloop_address(&loop);
    loop.set_sender_address(&sender);
    int port = 3333;
    if (argc == 2)
        port = std::atoi(argv[1]);
    sender.start(port);
    return 0;
}