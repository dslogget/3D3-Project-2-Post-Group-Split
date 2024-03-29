#include "Router.h"

void clearScreen(uint8_t mode) {
    std::cout << "\e[2J";
    if(mode) {
        std::cout << "\e[3J";
    }

    std::cout << "\e[;H\e[97m";
    std::cout.flush();
}



Packet::Packet(const std::vector<uint8_t>& data, int option)
    : stored_data(8, 0) {
    if(!option) {
        stored_data.insert(stored_data.end(), data.begin(), data.end());
    } else {
        stored_data.insert(stored_data.end(), data.begin() + 8, data.end());
    }
    type = &stored_data.at(0);
    id = &stored_data.at(1);
    dest_port = (uint16_t*)&stored_data.at(2);
    ip = (uint32_t*)&stored_data.at(4);
}

Packet::Packet(const Packet& tocpy)
    : stored_data(tocpy.getStoredVector()) {
    type = &stored_data.at(0);
    id = &stored_data.at(1);
    dest_port = (uint16_t*)&stored_data.at(2);
    ip = (uint32_t*)&stored_data.at(4);
}



Packet::~Packet() {


}

const std::vector<uint8_t>& Packet::getStoredVector() const {
    return stored_data;
}










Router::Router(std::string domain, uint8_t id, std::string initpath)
    : domain(domain), id(id) {
    //Initialise routing table/distance vector from file in "initpath"
    //Initialise id
    //allocate new socket
    //initialise sockets and call their socket handler

    std::string path;
    path += "logfile-";
    path += id;
    path += ".log";
    myLogFile.open(path, std::ios::app);


    std::cout << "\e]0;" << id << '\a';
    clearScreen(1);
    lastPrinted = std::time(0);
    lastPrinted_in_log = std::time(0);

    std::vector<std::tuple<uint8_t, uint8_t, uint16_t, uint8_t>> filevec;

    std::ifstream file(initpath);

    while(!file.eof()) {
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

    for(auto& entry : filevec) {
        if(std::get<1>(entry) == id) {
            port = std::get<2>(entry);
            break;
        }
    }



    RoutingEntry routTmp;

    routTmp.cost = 0;
    routTmp.destination = id;
    routTmp.port_dest = port;
    routTmp.via = id;
    routTmp.port_direct = port;
    routingTable.push_back(routTmp);
    std::cout << "\e[33mInitialising Socket" << std::endl;

    socket = new UDPSocket(domain.c_str(), std::to_string(port).c_str(), 1);

    std::cout << "Sending initial vectors\e[0m" << std::endl;


    for(auto& iter : filevec) {
        if(std::get<0>(iter) == id) {
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


    std::cout << std::endl << "\e[36mInitial Table: " << std::endl;
    printRoutingTable();
    std::cout << "\e[0m";

    file.close();




    handleSocket();

}

Router::~Router() {
    delete socket;
    myLogFile.close();
}



void Router::printRoutingTable() {
    lastPrinted = std::time(0);
    std::cout << std::setfill('-') << std::setw(29) << "-" << std::setfill(' ') << std::endl;
    std::cout << std::left << std::setw(6) << "origin" << "|" << std::setw(6) << "dest" << "|" << std::setw(9)
              << "port dest" << "|" << std::setw(4) << "cost" << "|" << std::endl;

    std::cout << std::setfill('-') << std::setw(29) << "-" << std::setfill(' ') << std::endl;

    for(auto& iter : routingTable) {
        std::cout << std::setw(6) << id << "|" << std::setw(6) << iter.destination << "|" << std::setw(9)
                  << iter.port_dest << "|" << std::setw(4) << (uint16_t)iter.cost << "|" << std::endl;
    }
    std::cout << std::setfill('-') << std::setw(29) << "-" << std::setfill(' ') << std::endl;
}



void Router::handleSocket() {
    //This function will continuously read in data, and decide if it should forward it, or if it should give it to the
    //"updateDistanceVector" function
    std::time_t lastPushed = std::time(0);

    while(1) {
        std::vector<uint8_t> data = socket->Receive();
        std::time_t currtime = std::time(0);
        while(data.size() != 0) {
            if(data.at(0) == 0) {
                forwardDataPacket(data);
            } else {
                updateDistanceVector(data);
            }
            data = socket->Receive();
        }

        if(difftime(currtime, lastPushed) >= 5) {
            pushDistanceVector();
            lastPushed = currtime;
            handleTimeouts();
        }
        if(difftime(currtime, lastPrinted) >= 7) {
            clearScreen();
            std::cout << "\e[96mRouting Table settled" << std::endl;
            printRoutingTable();
            std::cout << "\e[0m";
            std::cout.flush();
        }

    }
}

void Router::handleTimeouts() {
    std::time_t curr = std::time(0);
    std::vector<uint8_t> message(2);

    message.push_back(id);
    message.push_back(0);
    message.push_back(0);
    message.push_back(infty);






    for(unsigned int i = 0; i < timeouts.size(); i++) {

//        iostream_mutex.lock();
//        std::cout << (*iter).port << " " << std::difftime(curr, (*iter).lastHeardFrom) << std::endl;
//        iostream_mutex.unlock();



        if(std::difftime(curr, timeouts.at(i).lastHeardFrom) >= 10) {
            clearScreen();
            std::cout << "\e[31m" << timeouts.at(i).id << " is dead" << std::endl;
            removeRouter(timeouts.at(i).id);
            printRoutingTable();
            std::cout << "\e[0m";

            message.at(4) = timeouts.at(i).id;
            Packet pkt(message);
            *pkt.dest_port = port;
            *pkt.id = id;
            *pkt.type = 4;

            logToFile(pkt.getStoredVector());
            timeouts.erase(timeouts.begin() + i);
            i--;
        }
    }

    for(unsigned int i = 0; i < routingTable.size(); i++) {
        auto& entry = routingTable.at(i);

        if(entry.cost >= infty){
            if(difftime(curr, entry.timeInfty) >= 15){

                removeRouter(entry.destination);
                message.at(4) = entry.destination;
                Packet pkt(message);
                *pkt.dest_port = port;
                *pkt.id = id;
                *pkt.type = 4;
                logToFile(pkt.getStoredVector());
                i--;
            }
        } else {
            entry.timeInfty = curr;
        }

    }


}

void Router::logToFile(const std::vector<uint8_t>& distanceVector) {
    //Print the distance vector that caused the change,
    //the origin ID of that distance vector
    //A timestamp, and the
    //final routing table to the log

//    For assessment purposes each routing process should print the routing table to an output file, but only when
//    there has been a change in the routing table. If an arriving DV advertisement causes a change in the routing
//    table, then you should print out the timestamp, the routing table before the change, the DV that caused the
//    change (including the neighbor it came from), and the new routing table. If an arriving DV does not cause a
//    change in the routing table, then you do not need to print anything (though while you?re debugging, you may
//    wish to print out the routing table for each arriving DV). In the code that you hand in you should only print
//    a routing table for those DV?s that cause a change in the routing table.




    lastPrinted_in_log = std::time(0);

    if(distanceVector.at(0) != 4) {
        myLogFile
                << "\r\nOrigin ID: " << distanceVector.at(10) << "\r\n"
                << "Timestamp: " << std::asctime(std::localtime(&lastPrinted_in_log)) << "\r" << std::endl;

        uint8_t fillwidth = (distanceVector.size() - 10)/2;
        fillwidth *= 4;
        fillwidth += 9;
        myLogFile << std::setfill('-') <<  std::left << std::setw(fillwidth) << "-" << std::setfill(' ') << "\r" << std::endl;


        //store distance vector
        myLogFile << "ID:    |";

        for(unsigned int i=10; i<distanceVector.size(); i+=2){
            myLogFile << std::setw(4) << distanceVector.at(i)  << "|";
        }
        myLogFile << "\r\n";
        myLogFile << "Cost:  |" << std::setw(5);

        for(unsigned int i=11; i<distanceVector.size(); i+=2){
            myLogFile << std::setw(4) << (uint16_t)distanceVector.at(i) << "|";
        }
        myLogFile << "\r\n" << std::setw(5);


        myLogFile << std::setfill('-') << std::setw(fillwidth) << "-" << std::setfill(' ') << "\r\n\r" << std::endl;
        myLogFile << std::setfill('-') << std::setw(29) << "-" << std::setfill(' ') << "\r" << std::endl ;

    } else {
        myLogFile << "Router " << distanceVector.at(12) << "died" << "\r" << std::endl;
        myLogFile << "Timestamp: " << std::asctime(std::localtime(&lastPrinted_in_log)) << "\r" << std::endl;
    }

    //store routing table
    myLogFile << std::left << std::setw(6) << "origin" << "|" << std::setw(6) << "dest" << "|" << std::setw(9)
              << "port dest" << "|" << std::setw(4) << "cost" << "|" << "\r" << std::endl;

    myLogFile << std::setfill('-') << std::setw(29) << "-" << std::setfill(' ') << "\r" << std::endl ;

    for(auto& iter : routingTable) {
        myLogFile << std::setw(6) << id << "|" << std::setw(6) << iter.destination << "|" << std::setw(9)
                  << iter.port_dest << "|" << std::setw(4) << (uint16_t)iter.cost << "|" << "\r" << std::endl;
    }
    myLogFile << std::setfill('-') << std::setw(29) << "-" << std::setfill(' ') << "\r" << std::endl ;

}

void Router::updateDistanceVector(std::vector<uint8_t>& data) {
    //We need to update our distance vector
    //If a new router connects, we need to add it to our distance vector
    //an update will contain the origin router's ID, the origin port then each pair of destinations and costs

    //type(1) + destid(1) + destport(2) + ip(4) = 8
    //originPort(2) + id(1) + cost(1) = 4
    //total size of at least 12
    //std::cout << "Distance Vector Rec" << std::endl;


    //Check to see if the datapacket is of min length (And divisible by 2 to prevent Seg Faults)
    if(data.size() >= 12 && !(data.size() % 2)) {

        //Keep track of if routing table changes
        uint8_t routingTable_updated = 0;
        //The port the data came from
        uint16_t* originPort;
        originPort = (uint16_t*)&data.at(8);

        //The id the data came from
        uint8_t& originID = data.at(10);

        //current time for timeout updates
        std::time_t curr_time = std::time(0);

        //ptr to see if the timeout is found
        Timeout* tm = 0;
        for(auto& entry : timeouts) {
            if(entry.id == originID) {
                //if exists update last heard from
                entry.lastHeardFrom = curr_time;
                tm = &entry;
                break;
            }
        }

        //otherwise add a new timeout
        if(!tm) {
            Timeout newTm;
            newTm.id = originID;
            newTm.lastHeardFrom = std::time(0);
            timeouts.push_back(newTm);
        }


        //if a router died
        if(data.at(0) == 4) {
            //remove from table
            removeRouter(data.at(12));
            clearScreen();
            std::cout << "\e[31m" << data.at(12) << " has reportedly died" << std::endl;
            printRoutingTable();
            std::cout << "\e[0m";
            routingTable_updated = 1;

        } else {
            //otherwise begin to check for updates
            for(auto iter = data.begin() + 8 + 2; iter != data.end(); iter += 2) {

                uint8_t& dataID = *iter; //Id the data is referencing
                uint8_t& dataCost = *(iter + 1); //cost of the path to id

                //if this id isn't us
                if(dataID != id) {
                    //find the destination and origin in the routing table
                    RoutingEntry* routing = 0;
                    RoutingEntry* origin = 0;
                    for(auto& entry : routingTable) {
                        if(entry.destination == dataID) {
                            routing = &entry;
                            break;
                        }
                    }
                    //if not not in routign table
                    if(!routing) {
                        //add it
                        RoutingEntry tmp;
                        tmp.destination = dataID;
                        tmp.cost = infty;
                        tmp.port_dest = *originPort;
                        tmp.via = originID;
                        routingTable.push_back(tmp);
                        routing = &routingTable.back();

                        clearScreen();
                        std::cout << "\e[32mNew Node " << tmp.destination << std::endl;
                        printRoutingTable();
                        std::cout << "\e[0m";
                        routingTable_updated = 1;
                    }

                    for(auto& entry : routingTable) {
                        if(entry.destination == originID) {
                            origin = &entry;
                            break;
                        }
                    }


                    //if the cost is lower
                    if(dataCost + origin->cost < routing->cost) {
                        //update to use route
                        routing->cost = dataCost + origin->cost;
                        routing->via = originID;
                        routing->port_dest = *originPort;
                        clearScreen();
                        std::cout << "\e[32m" << "Improvement via " << routing->via << std::endl;
                        printRoutingTable();
                        std::cout << "\e[0m";
                        routingTable_updated = 1;
                    } else if(dataCost + origin->cost > routing->cost && dataCost < infty) {
                        //if it's higher
                        if(routing->via == originID) {
                            //and the current route to this note is through the origin (the DV), then set the cost
                            //to infinity to advoid coutn to infinity
                            clearScreen();
                            std::cout << "\e[31m" << "But wait, it gets worse!" << std::endl;
                            routing->cost = infty;
                            printRoutingTable();
                            std::cout << "\e[0m";
                            routingTable_updated = 1;
                        }

                    }






                } else if(data.at(0) > 1) {
                    //if the entry is referencing us
                    //and the type is 2 or 3
                    //find origin in routing table
                    RoutingEntry* origin = 0;
                    for(auto& entry : routingTable) {
                        if(entry.destination == originID) {
                            origin = &entry;
                            break;
                        }
                    }
                    //if it exists, and it always should
                    if(origin) {
                        //set the costs to the new connection param
                        origin->directCost = dataCost;
                        origin->port_direct = *originPort;

                        //if the type is 2, handshake with 3 back to confirm connection
                        if(data.at(0) == 2) {
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
                    routingTable_updated = 1;

                }
            }

        }

        //check if the initial link is better for all paths. if so use it.
        for(unsigned int i = 0; i < routingTable.size(); i++) {
            auto& entry = routingTable.at(i);
            //Prioritise lowest hop count
            if(entry.cost > entry.directCost && (entry.via != entry.destination || data.at(0) > 1)) {
                entry.port_dest = entry.port_direct;
                entry.via = entry.destination;
                entry.cost = entry.directCost;
                clearScreen();
                std::cout << "\e[33mUsing direct link" << std::endl;
                printRoutingTable();
                std::cout << "\e[0m";
                routingTable_updated = 1;
            }
        }

        //if there was an update, print to logs.
        if(routingTable_updated) {
            logToFile(data);
        }


    }
}
void Router::pushDistanceVector() {

    std::vector<uint8_t> message(2);
    uint16_t* portptr = (uint16_t*)&message.at(0);
    *portptr = port;

    for(auto& entry : routingTable) {
        message.push_back(entry.destination);
        message.push_back(entry.cost);
    }

    Packet pkt(message);
    for(auto& entry : routingTable) {
        if(entry.destination != id && entry.port_direct != 0) {
            *pkt.dest_port = entry.port_direct;
            *pkt.id = entry.via;
            *pkt.type = 1;
            socket->Send(pkt.getStoredVector(), domain.c_str(), *pkt.dest_port);
        }
    }

}

void Router::forwardDataPacket(std::vector<uint8_t>& data) {
    //this just simply pushes the datapacket based on the routing table
    if(data.at(1) != id) {
        for(unsigned int i = 0; i < routingTable.size(); i++) {
            if(routingTable.at(i).destination == data.at(1)) {
                lastPrinted_in_log = std::time(0);
                myLogFile << "Timestamp: " << std::asctime(std::localtime(&lastPrinted_in_log)) << "\r" << std::endl;
                myLogFile << "Forwarding to " << routingTable.at(i).via << "\r" << std::endl;
                std::cout << "Forwarding to " << routingTable.at(i).via << std::endl;
                socket->Send(data, domain.c_str(), routingTable.at(i).port_dest);
                break;
            }
        }
    } else {
        lastPrinted_in_log = std::time(0);
        myLogFile << "Timestamp: " << std::asctime(std::localtime(&lastPrinted_in_log)) << "\r" << std::endl;
        myLogFile << "Data received" << "\r" << std::endl;
        myLogFile << &data.at(8) << "\r" << std::endl;
        std::cout << "Data received" << std::endl;
        std::cout << &data.at(8) << std::endl;
    }
}


void Router::removeRouter(uint8_t del_id) {
    bool erased = false;
    for(unsigned int i = 0; i < routingTable.size(); i++) {
        if(routingTable.at(i).destination == del_id) {
            routingTable.erase(routingTable.begin() + i);
            i--;
            erased = true;
        } else if(routingTable.at(i).via == del_id) {
            routingTable.at(i).cost = infty;
        }
    }

    if(erased) {
        std::vector<uint8_t> message(2);
        uint16_t* portptr = (uint16_t*)&message.at(0);
        *portptr = port;

        message.push_back(id);
        message.push_back(0);

        message.push_back(del_id);
        message.push_back(infty);


        Packet pkt(message);
        for(auto& entry : routingTable) {
            if(entry.destination != id && entry.port_direct != 0) {
                *pkt.dest_port = entry.port_direct;
                *pkt.id = entry.via;
                *pkt.type = 4;
                socket->Send(pkt.getStoredVector(), domain.c_str(), *pkt.dest_port);
            }
        }
    }


}


