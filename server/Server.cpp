

//
// Created by jakub on 11.10.2021.
//

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <map>

#include "sockets.h"

using namespace std;

int main() {

    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);

    int number_of_connections = 100;

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);

    if (bind(socketFD, (sockaddr *) &addr, sizeof(addr))) {
        cout << "failed to bind socket";
        return -1;
    } else
        cout << "socket is bound" << endl;

    sockaddr_in client{};
    socklen_t socklen = sizeof(socklen);

    char client_id[40];
    char response[40];

    map<string, int> clients_datagram_count;

    while (number_of_connections) {

        recvfrom(socketFD, client_id, sizeof(client_id), 0, (sockaddr*) &client, &socklen);
        cout << "dgram from client: " << client_id << " #"<< 100 - number_of_connections + 1 << endl;
        clients_datagram_count[client_id]++;
        number_of_connections--;

        if (number_of_connections == 0)
            strcpy(response, "LAST");
        else
            strcpy(response, client_id);
        sendto(socketFD, response, sizeof(response), 0, (sockaddr*) &client, sizeof(client));
    }

    for (const auto& client_record : clients_datagram_count) {
        cout << "client nr :" + client_record.first + "sent :" << client_record.second << endl;
    }
    return 0;
}