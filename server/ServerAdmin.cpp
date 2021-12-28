//
// Created by jzielins on 13.12.2021.
//
#include "sockets.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;

void find_record(char *request, char *response){
    strcpy(response, "USER AUTHORIZED");
}

void handle_command(char *request, char *response) {
    cout << "command handled : " << request << endl;
    strcpy(response, "HANDLED");
}

int bind_socket(int protocol_type, in_addr_t address, int port, sockaddr_in& local_address){
    int n_socket = socket(AF_INET, protocol_type, 0);
    local_address.sin_family =  AF_INET;
    local_address.sin_port = htons(port);
    local_address.sin_addr.s_addr = address;

    if (bind(n_socket, (sockaddr *) &local_address, sizeof(local_address))) {
        cout << "failed to bind command socket";
        return -1;
    } else
        cout << "command socket is bound" << endl;
    return n_socket;
}


int main(){
    sockaddr_in server_admin_address{};
    int admin_socket = bind_socket(SOCK_STREAM, INADDR_ANY, ADMIN_PORT, server_admin_address);

    sockaddr_in server_cmd_address{};
    int cmd_socket = bind_socket(SOCK_DGRAM, INADDR_ANY, CMD_PORT, server_cmd_address);

    sockaddr_in admin_client_address{};
    socklen_t admin_socklen = sizeof(admin_client_address);

    sockaddr_in msg_server{};
    socklen_t socklen = sizeof(msg_server);

    char request[40];
    char response[40];
    char command[40];
    char command_response[40];

    int queue_size = 20;
    if (listen(admin_socket, queue_size)){
        cout <<  "failed to listen to a port";
        return errno;
    } else
        cout << "listening to a port : " <<  ADMIN_PORT << endl;

    fd_set server_sockets, ready_sockets;
    FD_ZERO(&server_sockets);
    FD_SET(cmd_socket , &server_sockets);


    while (true) {
        int connection_socket = accept(admin_socket, (sockaddr *) &msg_server, &socklen);

        if (connection_socket != -1) {
            FD_SET(connection_socket, &server_sockets);

            cout << "connected from : " << inet_ntoa(msg_server.sin_addr) << endl;
            cout << "waiting for queries..." << endl;

            while (true) {
                ready_sockets = server_sockets;
                if (select(FD_SETSIZE, &ready_sockets, nullptr, nullptr, nullptr) < 0){
                    cout << "select fail, errno : " << endl;
                    return errno;
                }
                //TODO  add fdset for writing sockets
                if (FD_ISSET(connection_socket,  &ready_sockets)){
                    long status = recv(connection_socket, request, sizeof(request), 0);
                    find_record(request, response);
                    cout << "query answered" << endl;
                    send(connection_socket, response, sizeof(response), 0);

                    if (status < 0)
                        break;
                }
                if (FD_ISSET(cmd_socket, &ready_sockets)){
                    recvfrom(cmd_socket, command, sizeof(command), 0, (sockaddr*) &admin_client_address, &admin_socklen);
                    handle_command(command,command_response);
                    sendto(cmd_socket, command_response, sizeof(command_response), 0, (sockaddr*) &admin_client_address, admin_socklen);
                }
            }
            FD_CLR(connection_socket, &server_sockets);
            close(connection_socket);
        } else {
            cout << "connection to msg_server failed. Errno : " << errno << endl;
            break;
        }
    }
    close(cmd_socket);
    close(admin_socket);
    return 0;
}
