#ifndef ROUTER_H
#define ROUTER_H

#include "Socketwrapper.h"
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
//For delays apparently std::this_thread::sleep_for(std::chrono::milliseconds(x)); should work

#define infty 128



/**
 *  @brief  This class makes it easier to construct a data packet for sending
 **/
class Packet {

private:
    std::vector<uint8_t> stored_data;


public:
    /**
     *  @class
     *  @brief  Initialisation with data
     *
     *  @param data - the data to be put into the payload of the packet
     *  @param option [= 0] - if 0 it creates a new packet, if one, it treats data like a packet
     **/
    Packet(const std::vector<uint8_t>& data, int option = 0);

    /**
     *  @class
     *  @brief  Copy constructor
     *
     *  @param tocpy - a copy constructor for the packet class
     **/
    Packet(const Packet& tocpy);


    ~Packet();

    /**
     *  @class
     *  @brief  Returns raw vector of packet
     *
     *  @return Packet Vector - contains the raw data of a packet
     **/
    const std::vector<uint8_t>& getStoredVector() const;


    /** @var    the type of tramission, should be 0 for data */
    uint8_t* type;
    /** @var    the destination id for the tramission */
    uint8_t* id;
    /** @var    the destination port for the tramission */
    uint16_t* dest_port;
    /** @var    the destination IP for the tramission */
    uint32_t* ip;
};

void clearScreen(uint8_t mode = 0);




/**
 *  @brief  This is the main class for the program, it handles all threads and communication
 **/
class Router {
public:

    /**
     *  @class
     *  @brief  Initialisation given domain, id, and initialisation file path
     *
     *  @param  domain - will be resolved into the IP the router is listening on
     *
     *  @param  id - the unique id assigned to the router
     *
     *  @param  initpath - the path for the initialisation of the distance vector/routing table
     **/
    Router(std::string domain, uint8_t id, std::string initpath);//Daniel

    /**
     *  @brief  Destructor for the class
     **/
    ~Router();//Daniel

    //Todo: Daniel - Add debug commands

    void printRoutingTable();


private:
    /** @brief where the unresolved domain of the router is stored **/
    std::string domain;
    /** @brief where the port of the router is stored **/
    uint16_t port;

    UDPSocket* socket = 0;



    /**
     *  @brief the unique id of the router
     **/
    uint8_t id = 0;


    /**
     *  @brief a struct that contains a single routing entry
     **/
    struct RoutingEntry {
        /** @brief The final destination of the route **/
        uint8_t destination = 0;

        /** @brief the total cost of the route **/
        uint8_t cost = 0;

        /** @brief the port the route leave the router on **/
        uint8_t via = 0;

        /** @brief the port the next hop router is receiving on **/
        uint16_t port_dest = 0;

        uint8_t directCost = infty;

        uint16_t port_direct = 0;
    };

    struct Timeout {
        std::time_t lastHeardFrom;
        uint8_t id;
    };




    /**
     *  @brief  The function to handle the incoming data packets
     *
     *  This function's main job is to decide if an incoming packet is a data packet to be forwarded on, or an
     *  update for the distance vector/routing table
     *
     **/
    void handleSocket();//James

    void handleTimeouts();

    /**
     *  @brief  The function to handle updates to the distance vector/routing table
     *
     *  This function's main job is to update the routing table/distance vector according to the bellman-ford
     *  algorithm
     *
     *  @param  data - This is the raw data vector received in from the socket.
     *          It is not stored anywhere, so can be modified if needed
     **/
    void updateDistanceVector(std::vector<uint8_t>& data);//Daniel

    /**
     *  @brief  The function to push the distance vector to its neighbouring routers
     *
     *  This function's main job is push the current distance vector to its neighbours. This function will be
     *  called periodically as needed (Brief states every 5 seconds)
     **/
    void pushDistanceVector(); //Daniel

    /**
    *   @brief  The function to forward a data packet along the correct routing path based on the routing table
    **/
    void forwardDataPacket(std::vector<uint8_t>& data);//James



    void removeRouter(uint8_t id);


    std::time_t lastPrinted;



    /** @brief a vector containing routing entries **/
    std::vector<struct RoutingEntry> routingTable;

    std::vector<struct Timeout> timeouts;


};

#endif // ROUTER_H
