#ifndef SOCKETWRAPPER_H
#define SOCKETWRAPPER_H

#include <string.h>
#include <thread>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <fstream>
#include <sstream>


/**
 *  @brief  this class is an encapsulation of the UDP BSD socket interface for Linux.
 *
 *  It allows the user to initialise the socket with any valid port and domain
 **/

class UDPSocket {
public:
    /**
     *  @class
     *  @brief  This is the constructor for the socket taking a domain and port
     *
     *  @param  domain - C-String - this parameter tells the socket which IP to bind on to
     *
     *  @param  port - C-String - this paramter tells the socket which port to bind on to
     *  @param  timeout [= 0] - how long before the socket blocks before proceeding if no transmission found, in seconds.
     **/
    UDPSocket(const char* domain, const char* port, unsigned int timeout = 0);

    /**
     *  @brief  destructor for the socket
     **/
    ~UDPSocket();



    /**
     *  @brief  this function returns the raw vector of data received by the socket.
     *
     *  @return A vector of the data received on the socket.
     **/
    std::vector<uint8_t> Receive();

    /**
     *  @brief  this function send a raw vector of data through the socket.
     *
     *  @param toSend - the vector of the data to be sent on the socket.
     *
     *  @param domain - C-String - the domain/IP the data is to be sent to.
     *
     *  @param port - C-String - the port the data is to be sent to.
     **/
    void Send(const std::vector<uint8_t>& toSend, const char* domain, const char* port);

    /**
     *  @brief  this function send a raw vector of data through the socket.
     *
     *  @param toSend - the vector of the data to be sent on the socket.
     *
     *  @param domain - C-String - the domain/IP the data is to be sent to.
     *
     *  @param port - the port the data is to be sent to.
     **/
    void Send(const std::vector<uint8_t>& toSend, const char* domain, uint16_t port);


    /**
     *  @brief  this function send a raw vector of data through the socket.
     *
     *  @param toSend - the vector of the data to be sent on the socket.
     *
     *  @param dest_info - The sockaddr of the destination that the socket is sending to.
     **/
    void Send(const std::vector<uint8_t>& toSend, const struct sockaddr_storage& dest_info);

    /**
     *  @brief  Gets the info of the last message received on the socket.
     *
     *  @return The address info of the last received transmission.
     **/
    const struct sockaddr_storage getReceivedInfo();
    /**
     *  @brief  Gets the port the socket is listening in host byte order.
     *
     *  @return The port the socket is listening in host byte order.
     **/
    uint16_t getSocketPort();


private:
    /** @brief the file descriptor of the socket **/
    int socketfd = 0;
    /** @brief the sockets addrinfo linked-list **/
    struct addrinfo *socket_info;
    /** @brief the info of the origin of the last received transmission **/
    struct sockaddr_storage rec_info;

protected:


};











#endif // SOCKETWRAPPER_H
