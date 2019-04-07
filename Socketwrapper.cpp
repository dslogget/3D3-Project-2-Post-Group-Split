#include "Socketwrapper.h"


UDPSocket::UDPSocket(const char* domain, const char* port, unsigned int timeout) {
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET; // don't care IPv4 or IPv6



    hints.ai_socktype = SOCK_DGRAM; // UDP sockets



    if((status = getaddrinfo(domain, port, &hints, &socket_info)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }


    socketfd = socket(socket_info->ai_family, socket_info->ai_socktype, socket_info->ai_protocol);
    if(socketfd == -1) {
        perror("socket error");
        exit(1);
    }

    bind(socketfd, socket_info->ai_addr, socket_info->ai_addrlen);
    if(socketfd == -1) {
        perror("bind error");
        exit(1);
    }

    if(timeout) {
        struct timeval timeoutval;
        timeoutval.tv_sec = timeout;
        timeoutval.tv_usec = 0;
        if(setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeoutval, sizeof(timeoutval)) < 0) {
            perror("control error");
            exit(1);
        }
    }

}


UDPSocket::~UDPSocket() {
    freeaddrinfo(socket_info); // free the linked-list
    shutdown(socketfd, 2);
}



std::vector<uint8_t> UDPSocket::Receive() {
    std::vector<uint8_t> toret(2000, 0);
    socklen_t addrlen = sizeof(rec_info);
    int newlen;
    if((newlen = recvfrom(socketfd, toret.data(), toret.size(), 0, (sockaddr*)&rec_info, &addrlen)) == -1 && errno != 11) {
        std::cout << errno << std::endl;
        perror("receive error");
        exit(1);
    }
    toret.resize(newlen == -1 ? 0 : newlen);
    return toret;
}

void UDPSocket::Send(const std::vector<uint8_t>& toSend, const char* domain, const char* port) {
    struct addrinfo *dest_info;
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET; // don't care only IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP sockets


    if((status = getaddrinfo(domain, port, &hints, &dest_info)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    if(sendto(socketfd, toSend.data(), toSend.size(), 0, dest_info->ai_addr, dest_info->ai_addrlen) == -1) {
        perror("send error");
        exit(1);
    }

    freeaddrinfo(dest_info);

}

void UDPSocket::Send(const std::vector<uint8_t>& toSend, const char* domain, uint16_t port) {
    struct addrinfo *dest_info;
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET; // don't care only IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP sockets


    if((status = getaddrinfo(domain, std::to_string(port).c_str(), &hints, &dest_info)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    if(sendto(socketfd, toSend.data(), toSend.size(), 0, dest_info->ai_addr, dest_info->ai_addrlen) == -1) {
        perror("send error");
        exit(1);
    }

    freeaddrinfo(dest_info);

}



void UDPSocket::Send(const std::vector<uint8_t>& toSend, const struct sockaddr_storage& dest_info) {
    socklen_t len = sizeof(dest_info);
    if(sendto(socketfd, toSend.data(), toSend.size(), 0, (const sockaddr*)&dest_info, len) == -1) {
        perror("send error");
        exit(1);
    }
}

const struct sockaddr_storage UDPSocket::getReceivedInfo() {
    return rec_info;
}


uint16_t UDPSocket::getSocketPort() {
    return ntohs(((sockaddr_in*)socket_info->ai_addr)->sin_port);
}




