//
// Created by jakub on 11.10.2021.
//
#include <iostream>
#include <unistd.h>
#include <cstring>

#include "sockets.h"

using namespace std;

int main(int argc, char** argv) {

    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

    sockaddr_in server_address{};

    server_address.sin_port = htons(CMD_PORT);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    char command[40];
    char response[40];

    while (true){
        cout <<  "Command to run on server\n";
        cin >> command;

        if (sendto(client_socket, command, sizeof(command), 0, (sockaddr*)&server_address, sizeof(server_address)) <= 0) {
            cout << "send / err :" << errno << endl;
            break;
        }
        else {
            cout << "command has been issued" << endl;
        }

        socklen_t server_address_len = sizeof(server_address);
        if (recvfrom(client_socket, response, sizeof(response), 0, (sockaddr*)&server_address, &server_address_len) <= 0 ) {

            switch(errno){
                case CONNECTION_REFUSED:
                    cout << "receive / connection refused" << endl;
                    break;
                case TIMEOUT:
                    cout << "receive / timeout" << endl;
                    break;
                default:
                    cout << "receive / unknown error : " << errno << endl;
            }
            break;
        }
        cout << "response from admin server :" << response << endl;
    }
    return 0;
}