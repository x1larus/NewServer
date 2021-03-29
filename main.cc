#include "socket_communication.h"

int main()
{
    socket_communication sender;
    sender.start(3333);
    return 0;
}