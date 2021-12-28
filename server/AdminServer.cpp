//
// Created by jzielins on 28.12.2021.
//
#include <iostream>
#include <unistd.h>
#include <cstring>

#include "AdminServer.h"

using namespace std;

AdminServer::AdminServer() {
    queue_size = 20;

    admin_server_address = inet_association(AF_INET, ADMIN_PORT, INADDR_ANY);
    server_cmd_address = inet_association(AF_INET, CMD_PORT, INADDR_ANY);

    msg_server_address_size = sizeof(msg_server_address);


    _request = vector<char>(50);
    _response = vector<char>(50);
    _command = vector<char>(50);
    _command_response = vector<char>(50);

    admin_server_active = true;
    connection_opened = false;
}

int AdminServer::run() {

    admin_socket = bind_socket(SOCK_STREAM, admin_server_address);
    cmd_socket = bind_socket(SOCK_DGRAM, server_cmd_address);

    if (listen(admin_socket, queue_size)){
        cout <<  "failed to listen to a port";
        return errno;
    } else
        cout << "listening to a port : " <<  ADMIN_PORT << endl;

    prepare_fdset();

    while (admin_server_active) {
        msg_server_connection_socket = accept(admin_socket, (sockaddr*) &msg_server_address, &msg_server_address_size);

        if (msg_server_connection_socket != -1) {
            connection_opened = true;
            handle_msg_server_connection();
        } else {
            handle_msg_server_failed_connection();
        }
    }

    close(cmd_socket);
    close(admin_socket);
    return 0;
}

void AdminServer::handle_msg_server_connection() {

    FD_SET(msg_server_connection_socket, &server_sockets);

    cout << "connected from : " << inet_ntoa(msg_server_address.sin_addr) << endl;
    cout << "waiting for queries..." << endl;

    while (connection_opened){
        ready_sockets = server_sockets;
        //TODO  add fdset for writing to sockets
        if (select(FD_SETSIZE, &ready_sockets, nullptr, nullptr, nullptr) < 0){
            cout << "select fail, errno : " << errno << endl;
            connection_opened = false;
        } else {
            if (FD_ISSET(msg_server_connection_socket,  &ready_sockets))
                handle_query();

            if (FD_ISSET(cmd_socket, &ready_sockets))
                handle_command_request();
        }
    }
    FD_CLR(msg_server_connection_socket, &server_sockets);
    close(msg_server_connection_socket);
}

void AdminServer::handle_query() {
    long status = recv(msg_server_connection_socket, _request.data(), _request.size(), 0);
    find_record(_request, _response);
    cout << "query answered" << endl;
    send(msg_server_connection_socket, _response.data(), _response.size(), 0);
}

void AdminServer::handle_command_request() {
    recvfrom(cmd_socket, _command.data(), _command.size(), 0, (sockaddr*) &admin_client_address, &admin_client_address_size);
    handle_command(_command, _command_response);
    sendto(cmd_socket, _command_response.data(), _command_response.size(), 0, (sockaddr*) &admin_client_address, admin_client_address_size);
}

void AdminServer::handle_msg_server_failed_connection() {
    cout << "connection to msg_server failed. Errno : " << errno << endl;
    cout << "waiting for next msg_server connection\n";
}

int AdminServer::bind_socket(int protocol_type, sockaddr_in &address_to_bind) {
    int n_socket = socket(AF_INET, protocol_type, 0);

    if (bind(n_socket, (sockaddr *) &address_to_bind, sizeof(address_to_bind))) {
        cout << "failed to bind command socket";
        return -1;
    } else
        cout << "command socket is bound" << endl;
    return n_socket;
}

void AdminServer::prepare_fdset() {
    FD_ZERO(&server_sockets);
    FD_SET(cmd_socket, &server_sockets);
}

sockaddr_in AdminServer::inet_association(sa_family_t in_family, in_port_t port, in_addr_t address) {
    sockaddr_in association{};
    association.sin_family = in_family;
    association.sin_port = port;
    association.sin_addr.s_addr = address;
    return association;
}

void AdminServer::find_record(std::vector<char> &request, std::vector<char> &response) {
    strcpy(request.data(), "USER AUTHORIZED");
}

void AdminServer::handle_command(std::vector<char> &request, std::vector<char> &response) {
    cout << "command handled : " << request.data() << endl;
    strcpy(response.data(), "HANDLED");
}