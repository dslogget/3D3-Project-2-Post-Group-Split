#include <iostream>
#include "Socketwrapper.h"

int main(int argc, char** arg)
{
    char arr[] = "Echo";
    char ip[] = "127.0.0.1";
    std::vector<uint8_t> message(arr, arr + strlen(arr));
    UDPSocket sock(ip, arg[1]);
    std::cout << sock.Receive().data() << std::endl;
    sock.Send(message, sock.getReceivedInfo());
    return 0;
}
