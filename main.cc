#include "socket_communication.h"
#include "ev_loop.h"
#include <cstdlib>

int main(int argc, char **argv)
{
    socket_communication sender;
    ev_loop loop;
    sender.set_evloop_address(&loop);
    loop.set_sender_address(&sender);
    sender.start(std::atoi(argv[1]));
    return 0;
}