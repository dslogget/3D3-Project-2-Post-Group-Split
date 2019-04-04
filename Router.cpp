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
    std::cout << "Initialising Socket" << std::endl;

    socket = new UDPSocket(domain.c_str(), std::to_string(port).c_str(), 5);

    std::cout << "Sending initial vectors" << std::endl;


    for(auto& iter : filevec){
        if(std::get<0>(iter) == id){
            std::vector<uint8_t> message(6);
            uint16_t* portptr = (uint16_t*)&message.at(0);
            *portptr = port;
            message.at(2) = id;
            message.at(3) = 0;
            message.at(4) = std::get<1>(iter);
            message.at(5) = std::get<3>(iter);
            Packet pkt(message);
            *pkt.dest_port = std::get<2>(iter);
            *pkt.id = id;
            *pkt.type = 2;
            socket->Send(pkt.getStoredVector(), domain.c_str(), *pkt.dest_port);
        }
    }


    std::cout << std::endl << "Initial Table: " << std::endl;
    for(auto& iter : routingTable){
        std::cout << iter.destination << std::endl;
        std::cout << (uint16_t)iter.cost << std::endl;
        std::cout << iter.port_dest << std::endl;
    }

    file.close();




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
    std::time_t lastPushed = std::time(0);

    while(1){
        std::vector<uint8_t> data = socket->Receive();

        if(data.size() != 0){
            if(data.at(0) == 0){
                forwardDataPacket(data);
            }else{
                updateDistanceVector(data);
            }
        }

        std::time_t currtime = std::time(0);
        if(difftime(currtime, lastPushed) >= 5){
            pushDistanceVector();
            lastPushed = currtime;
        }

    }
}

void Router::updateDistanceVector(std::vector<uint8_t>& data)
{
    //We need to update our distance vector
    //If a new router connects, we need to add it to our distance vector
    //an update will contain the origin router's ID, the origin port then each pair of destinations and costs

    //type(1) + destid(1) + destport(2) + ip(4) = 8
    //originPort(2) + id(1) + cost(1) = 4
    //total size of at least 12
    //std::cout << "Distance Vector Rec" << std::endl;

    if(data.size() >= 12 && !(data.size() % 2)){
        uint16_t* originPort;
        originPort = (uint16_t*)&data.at(8);
        uint8_t& originID = data.at(10);


        for(auto iter = data.begin() + 8 + 2; iter != data.end(); iter += 2){

            uint8_t& dataID = *iter;
            uint8_t& dataCost = *(iter + 1);

            if(dataID != id){
                RoutingEntry* routing = 0;
                RoutingEntry* origin = 0;
                for(auto& entry : routingTable){
                    if(entry.destination == dataID){
                        routing = &entry;
                        break;
                    }
                }

                if(!routing){
                    RoutingEntry tmp;
                    tmp.destination = dataID;
                    tmp.cost = 0xF0;
                    tmp.port_dest = *originPort;
                    tmp.via = originID;
                    routingTable.push_back(tmp);
                    routing = &routingTable.back();

                    std::cout << "New Node " << tmp.destination << std::endl;
                    for(auto& iter : routingTable){
                        std::cout << iter.destination << std::endl;
                        std::cout << (uint16_t)iter.cost << std::endl;
                        std::cout << iter.port_dest << std::endl;
                    }
                }

                for(auto& entry : routingTable){
                    if(entry.destination == originID){
                        origin = &entry;
                        break;
                    }
                }



                if(dataCost + origin->cost < routing->cost){
                    routing->cost = dataCost + origin->cost;
                    routing->via = originID;
                    routing->port_dest = *originPort;
                    std::cout << "Improvement" << std::endl;
                    for(auto& iter : routingTable){
                        std::cout << iter.destination << std::endl;
                        std::cout << (uint16_t)iter.cost << std::endl;
                        std::cout << iter.port_dest << std::endl;
                    }



                }else{

                    //if things get worse

                }






            }else if(data.at(0) > 1){
                RoutingEntry* origin = 0;
                for(auto& entry : routingTable){
                    if(entry.destination == originID){
                        origin = &entry;
                        break;
                    }
                }
                if(origin){
                    origin->directCost = dataCost;
                    origin->port_direct = *originPort;

                    if(data.at(0) == 2){
                        std::vector<uint8_t> message(6);
                        uint16_t* portptr = (uint16_t*)&message.at(0);
                        *portptr = port;
                        message.at(2) = id;
                        message.at(3) = 0;
                        message.at(4) = originID;
                        message.at(5) = dataCost;
                        Packet pkt(message);
                        *pkt.dest_port = *originPort;
                        *pkt.id = originID;
                        *pkt.type = 3;
                        socket->Send(pkt.getStoredVector(), domain.c_str(), *pkt.dest_port);
                    }
                }

            }
        }
        for(auto& entry : routingTable){
            if(entry.cost > entry.directCost){
                entry.port_dest = entry.port_direct;
                entry.via = entry.destination;
                entry.cost = entry.directCost;
                std::cout << "Using direct link" << std::endl;
                for(auto& iter : routingTable){
                    std::cout << iter.destination << std::endl;
                    std::cout << (uint16_t)iter.cost << std::endl;
                    std::cout << iter.port_dest << std::endl;
                }
            }
        }



    }
}
void Router::pushDistanceVector()
{

    std::vector<uint8_t> message(2);
    uint16_t* portptr = (uint16_t*)&message.at(0);
    *portptr = port;

    for(auto& entry : routingTable){
        message.push_back(entry.destination);
        message.push_back(entry.cost);
    }

    Packet pkt(message);
    for(auto& entry : routingTable){
        *pkt.dest_port = entry.port_dest;
        *pkt.id = entry.via;
        *pkt.type = 1;
        socket->Send(pkt.getStoredVector(), domain.c_str(), *pkt.dest_port);
    }

}

void Router::forwardDataPacket(std::vector<uint8_t>& data)
{
    //this just simply pushes the datapacket based on the routing table
    if(data.at(1) != id){
        for(unsigned int i=0; i< routingTable.size(); i++){
            if(routingTable.at(i).destination == data.at(1)){
                std::cout << "Forwarding to " << routingTable.at(i).via << std::endl;
                socket->Send(data, domain.c_str(), routingTable.at(i).port_dest);
                break;
            }
        }
    }else{
        std::cout << "Data received" << std::endl;
        std::cout << &data.at(8) << std::endl;
    }
}
