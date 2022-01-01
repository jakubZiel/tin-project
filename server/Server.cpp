#include <iostream>
#include <unistd.h>
#include <cstring>
#include <map>

#include "sockets.h"
#include "Server.h"

using namespace std;

Server::Server() {

    server_address = associate_inet(AF_INET, SERVER_PORT, INADDR_ANY);
    admin_server_address = associate_inet(AF_INET, ADMIN_PORT, inet_addr("127.0.0.1"));

    client_id = vector<char>(50);
    response = vector<char>(50);
    admin_query = vector<char>(50);
    admin_response = vector<char>(50);

    clients_datagram_count = map<string, int>(); // TODO is it necessary?

}

sockaddr_in Server::associate_inet(sa_family_t in_family, in_port_t port, in_addr_t address) {
    sockaddr_in association{};
    association.sin_family = in_family;
    association.sin_port = htons(port);
    association.sin_addr.s_addr = address;
    return association;
}

int Server::connect_to_admin() {
    admin_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family =  AF_INET;
    address.sin_port = htons(ADMIN_PORT);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connection_status = connect(admin_socket, (sockaddr*) &admin_server_address, sizeof(admin_server_address));
    if (connection_status == -1){
        cout << "Failed to connect to admin server, errno: " << errno << endl;
        return errno;
    }
    cout << "Connected to admin server." << endl;
    return admin_socket;
}

int Server::bind_socket(int protocol_type, sockaddr_in &address_to_bind) {
    int n_socket = socket(AF_INET, protocol_type, 0);

    if (bind(n_socket, (sockaddr *) &address_to_bind, sizeof(address_to_bind))) {
        cout << "Failed to bind socket, errno: " << errno;
        return -1;
    } else
        cout << "Socket is bound." << endl;
    return n_socket;
}

size_t Server::query_admin(char* query){
    int request_size;
    send(admin_socket, query, sizeof(query), 0);
    return recv(admin_socket, admin_response.data(), admin_response.size(), 0);
}

int Server::run() {

    admin_socket = connect_to_admin();

    server_socket = bind_socket(SOCK_DGRAM, server_address);

    sockaddr_in client{};
    socklen_t socklen = sizeof(client);

    while (number_of_connections) {
        //TODO add signalfd , add select
        recvfrom(server_socket, client_id.data(), client_id.size(), 0, (sockaddr*) &client, &socklen);
        cout << "Datagram from client: " << client_id.data() << " #"<< 100 - number_of_connections + 1 << endl;
        clients_datagram_count[client_id.data()]++;
        number_of_connections--;

        strcpy(admin_query.data(), "SOME_QUERY");
        query_admin(admin_query.data());

        if (number_of_connections == 0)
            strcpy(response.data(), "LAST");
        else
            strcpy(response.data(), client_id.data());
        sendto(server_socket, response.data(), response.size(), 0, (sockaddr*) &client, sizeof(client));
    }

    for (const auto& client_record : clients_datagram_count) {
        cout << "Client nr:" + client_record.first + " sent:" << client_record.second << endl;
    }
    return 0;
}

int main() {
    Server server;
    server.run();
}

