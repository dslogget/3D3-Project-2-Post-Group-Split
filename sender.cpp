#include <iostream>
#include "Socketwrapper.h"
#include "Router.h"

int main(int argc, char** arg)
{
    char ip[] = "127.0.0.1";
    std::string tststr = "Testing 123";
    std::vector<uint8_t> message(tststr.begin(), tststr.end());

    Packet packet(message);
    *packet.type = 0;
    *packet.id = arg[3][0];
    std::cout << (uint16_t)packet.getStoredVector().at(0) << std::endl;
    std::cout << (uint16_t)*packet.type << std::endl;
    UDPSocket sock(ip, arg[1]);
    sock.Send(packet.getStoredVector(), ip, arg[2]);
    return 0;
}
