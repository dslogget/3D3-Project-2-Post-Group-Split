#include "Router.h"


Packet::Packet(const std::vector<uint8_t>& data, int option)
    :stored_data(8, 0){
    if(!option){
        stored_data.insert(stored_data.end(), data.begin(), data.end());
    }else{
        stored_data.insert(stored_data.end(), data.begin() + 8, data.end());
    }
    type = &stored_data.at(0);
    id = &stored_data.at(1);
    dest_port = (uint16_t*)&stored_data.at(2);
    ip = (uint32_t*)&stored_data.at(4);
}

Packet::Packet(const Packet& tocpy)
    :stored_data(tocpy.getStoredVector()){
    type = &stored_data.at(0);
    id = &stored_data.at(1);
    dest_port = (uint16_t*)&stored_data.at(2);
    ip = (uint32_t*)&stored_data.at(4);
}



Packet::~Packet(){


}

const std::vector<uint8_t>& Packet::getStoredVector() const{
    return stored_data;
}










Router::Router(std::string domain, uint8_t id, std::string initpath)
:domain(domain), id(id)
{
    //Initialise routing table/distance vector from file in "initpath"
    //Initialise id
    //allocate new socket
    //initialise sockets and call their socket handler

    std::vector<std::tuple<uint8_t, uint8_t, uint16_t, uint8_t>> filevec;

    std::ifstream file(initpath);

    while(!file.eof()){
        std::string buf;

        std::getline(file, buf, ',');
        uint8_t src = buf[0];
        //std::cout << src << std::endl;

        std::getline(file, buf, ',');
        uint8_t dest = buf[0];
        //std::cout << dest << std::endl;

        std::getline(file, buf, ',');
        uint16_t port = std::stoi(buf);
        //std::cout << port << std::endl;

        std::getline(file, buf, '\n');
        uint8_t cost = std::stoi(buf);
        //std::cout << (uint16_t)src << std::endl;


        filevec.push_back(std::make_tuple(src, dest, port, cost));

    }

    for(auto& entry : filevec){
        if(std::get<1>(entry) == id){
            port = std::get<2>(entry);
            break;
        }
    }



    RoutingEntry routTmp;

    routTmp.cost = 0;
    routTmp.destination = id;
    routTmp.port_dest = port;
    routingTable.push_back(routTmp);

    std::cout << "Initialising Table" << std::endl;


    for(auto& iter : filevec){
        if(std::get<0>(iter) == id){
            routTmp.cost = std::get<3>(iter);
            routTmp.destination = std::get<1>(iter);
            routTmp.port_dest = std::get<2>(iter);
            routingTable.push_back(routTmp);
        }
    }


    std::cout << std::endl << "Initial Table: " << std::endl;
    for(auto& iter : routingTable){
        std::cout << iter.destination << std::endl;
        std::cout << (uint16_t)iter.cost << std::endl;
        std::cout << iter.port_dest << std::endl;
    }

    file.close();

    socket = new UDPSocket(domain.c_str(), std::to_string(port).c_str(), 5);


    handleSocket();

}

Router::~Router()
{
    delete socket;

}




void Router::handleSocket()
{
    //This function will continuously read in data, and decide if it should forward it, or if it should give it to the
    //"updateDistanceVector" function


    while(1){

        std::vector<uint8_t> data = socket->Receive();

        if(data.size() != 0){

            if(data.at(0) == 0){
                forwardDataPacket(data);
            }

            else{
                updateDistanceVector(data);
            }
        }
    }
}

void Router::updateDistanceVector(std::vector<uint8_t>& data)
{
    //We need to update our distance vector
    //If a new router connects, we need to add it to our distance vector

    //an update will contain the origin router's ID, then each pair of destinations and costs, and be terminated by a null.
}
void Router::pushDistanceVector()
{
    //we need to push the current distance vector to all neighbours.
}

void Router::forwardDataPacket(std::vector<uint8_t>& data)
{
    //this just simply pushes the datapacket based on the routing table
    if(data.at(1) != id){
        for(unsigned int i=0; i< routingTable.size(); i++){
            if(routingTable.at(i).destination == data.at(1)){

                socket->Send(data, domain.c_str(), routingTable.at(i).port_dest);
                break;
            }
        }
    }else{
        std::cout << "Data received" << std::endl;
        std::cout << &data.at(8) << std::endl;
    }
}
