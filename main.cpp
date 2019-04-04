#include <iostream>
#include "Socketwrapper.h"

int main(int argc, char** arg)
{
    char ip[] = "127.0.0.1";
    UDPSocket sock(ip, arg[1]);
    strcpy(port, arg[2]);
    sock.Send("Test", ip, port);

    return 0;
}
