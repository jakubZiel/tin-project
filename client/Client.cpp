//
// Created by jakub on 11.10.2021.
//

#include <iostream>
#include <unistd.h>
#include <cstring>

#include "sockets.h"

using namespace std;

int main(int argc, char** argv) {

    if (argc < 2){
        cout << "missing client number";
        return -2;
    }
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

    sockaddr_in server_address{};

    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");


    char request_buffer[40];
    char is_last[40];
    strcpy(request_buffer, argv[1]);

    int counter = 0;
    while (true){

        sleep(1);
        if (sendto(client_socket, request_buffer, sizeof(request_buffer),  , (sockaddr*)&server_address, sizeof(server_address)) <= 0) {
            cout << "send / err :" << errno << endl;
            break;
        }
        else {
            counter++;
            cout << "wrote " << counter << endl;
        }
        socklen_t server_address_len = sizeof(server_address);
        if (recvfrom(client_socket, is_last, sizeof(is_last), 0, (sockaddr*)&server_address, &server_address_len) <= 0 ) {

            switch(errno){
                case CONNECTION_REFUSED:
                    cout << "receive / connection refused" << endl;
                    break;
                case TIMEOUT:
                    cout << "receive / timeout" << endl;
                    break;
                default:
                    cout << "receive / unknown error" << endl;
            }
             break;
        }
        cout <<"message for :" << is_last << endl;

    }
    cout << "client :" << request_buffer << " sent :" << counter;
    return 0;
}