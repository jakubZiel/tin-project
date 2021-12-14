//
// Created by jakub on 11.10.2021.
//

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <map>

#include "sockets.h"

using namespace std;

int connect_to_admin(){
    int admin_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_address{};
    server_address.sin_family =  AF_INET;
    server_address.sin_port = htons(ADMIN_PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connection_status = connect(admin_socket, (sockaddr*) &server_address, sizeof(server_address));
    if (connection_status == -1){
        cout << "failed to connect to admin server";
        return errno;
    }
    cout << "connected to admin server" << endl;
    return admin_socket;
}

size_t query_admin(int admin_socket, char* query, char* response){
    int request_size;
    send(admin_socket, query, sizeof(query), 0);
    return recv(admin_socket, response, sizeof(response), 0);
}

int main() {

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    int number_of_connections = 100;

    int admin_socket = connect_to_admin();

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);

    if (bind(socket_fd, (sockaddr *) &addr, sizeof(addr))) {
        cout << "failed to bind socket";
        return -1;
    } else
        cout << "socket is bound" << endl;

    sockaddr_in client{};
    socklen_t socklen = sizeof(client);

    char client_id[40];
    char response[40];

    map<string, int> clients_datagram_count;

    while (number_of_connections) {

        recvfrom(socket_fd, client_id, sizeof(client_id), 0, (sockaddr*) &client, &socklen);
        cout << "dgram from client: " << client_id << " #"<< 100 - number_of_connections + 1 << endl;
        clients_datagram_count[client_id]++;
        number_of_connections--;

        char admin_query[40];
        strcpy(admin_query, "SOME_QUERY");
        char admin_response[40];
        query_admin(admin_socket, admin_query, admin_response);

        if (number_of_connections == 0)
            strcpy(response, "LAST");
        else
            strcpy(response, client_id);
        sendto(socket_fd, response, sizeof(response), 0, (sockaddr*) &client, sizeof(client));
    }

    for (const auto& client_record : clients_datagram_count) {
        cout << "client nr :" + client_record.first + "sent :" << client_record.second << endl;
    }
    return 0;
}