#include "socket_communication.h"
#include "ev_loop.h"

int main()
{
    socket_communication sender;
    ev_loop loop;
    sender.set_evloop_address(&loop);
    loop.set_sender_address(&sender);
    sender.start(3333);
    return 0;
}