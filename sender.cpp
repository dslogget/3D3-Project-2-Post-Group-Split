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
    UDPSocket sock(ip, arg[1]);
    sock.Send(packet.getStoredVector(), ip, arg[2]);

    std::cout << "\e[32mPacket Sent!\e[0m" << std::endl;
    return 0;
}
