#include <iostream>
#include <unistd.h>
#include <cstring>
#include <map>
#include <sys/signalfd.h>
#include <csignal>

#include "sockets.h"
#include "Server.h"

using namespace std;

Server::Server() {

    server_address = associate_inet(AF_INET, SERVER_PORT, INADDR_ANY);
    admin_server_address = associate_inet(AF_INET, ADMIN_PORT, inet_addr("127.0.0.1"));

    client_message = vector<char>(2000); // TODO put size in a constant
    response = vector<char>(2000);
    admin_query = vector<char>(1000);
    admin_response = vector<char>(1000);

    clients_datagram_count = map<string, int>(); // TODO is it necessary?
    server_active = true;

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

    prepare_fdset();
    prepare_signal_fd();

    while (server_active) {
        //TODO add signalfd , add select
        ready_sockets = sockets;
        if (select(FD_SETSIZE, &ready_sockets, nullptr, nullptr, nullptr) < 0) {
            cout << "Select fail, errno: " << errno << endl;
            exit(errno);

        } else if (FD_ISSET(signal_fd, &ready_sockets)){
          handle_interrupt();

        } else if (FD_ISSET(server_socket, &ready_sockets)) {
            recvfrom(server_socket, client_message.data(), client_message.size(), 0, (sockaddr *) &client, &socklen);
            auto clientInfo =  ClientInfo(client);

            cout << "Datagram from client: " << client_message.data() << endl;
            Message message;
            try {
                message = Message(client_message.data());
            } catch (std::invalid_argument &e) {
                cerr << e.what() << endl;
                continue;
            }
            channels[message.channel].insert(clientInfo);
            cout << "client ports: "; // TODO debug, remove in the future
            for (auto& el : channels[message.channel]) { cout << el.addr.sin_port << " "; }
            cout << endl;
            clients_datagram_count[client_message.data()]++;

            strcpy(admin_query.data(), "SOME_QUERY");
            query_admin(admin_query.data());

            if (!message.is_listener) {
                strcpy(response.data(), message.message.c_str());
                for (auto& cl : channels[message.channel]) {
                    sendto(server_socket, response.data(), response.size(), 0, (sockaddr *) &cl.addr, sizeof(cl.addr));
                }
            }
        }
    }
    FD_CLR(server_socket, &sockets);
    close(server_socket);

    // TODO remove in the future, development purposes
    for (const auto& client_record : clients_datagram_count) {
        cout << "Client nr:" + client_record.first + " sent:" << client_record.second << endl;
    }
    return 0;
}

void Server::prepare_fdset() {
    FD_ZERO(&sockets);
    FD_SET(server_socket, &sockets);
}

void Server::prepare_signal_fd() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
        perror("can't block SIGINT for this process");
        exit(errno);
    }

    signal_fd = signalfd(-1, &mask, 0);

    if (signal_fd == -1){
        perror("can't create signal fd");
        exit(errno);
    }

    FD_SET(signal_fd, &sockets);
}

void Server::handle_interrupt() {
    siginfo_t signal_info;
    read(signal_fd, &signal_info, sizeof(signal_info));
    cout << endl << "Interrupted by SIGINT" << endl;
    cout << signal_info.si_errno << endl;

    server_active = false;
}

int main() {
    Server server;
    server.run();
    cout << "\nClosed gracefully...";
    return 0;
}

