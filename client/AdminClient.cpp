//
// Created by jakub on 11.10.2021.
//
#include <iostream>
#include <unistd.h>

#include "AdminClient.h"

using namespace std;

int main() {
    //cout << SO_SNDBUF << endl;

    AdminClient adminClient;
    adminClient.run();
    return 0;
}

AdminClient::AdminClient() {
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    server_address = inet_association(AF_INET, CMD_PORT, inet_addr("127.0.0.1"));

    _command = vector<char>(50);
    _response = vector<char>(50);
}

int AdminClient::init_socket(int protocol_type){
    client_socket = socket(AF_INET, protocol_type, 0);
}

sockaddr_in AdminClient::inet_association(sa_family_t in_family, in_port_t port, in_addr_t address){
    sockaddr_in association{};
    association.sin_family = in_family;
    association.sin_port = htons(port);
    association.sin_addr.s_addr = address;

    return association;
}

bool AdminClient::send_command() {
    cout <<  "Command to run on server\n";
    cin >> _command.data();

    if (sendto(client_socket, _command.data(), _command.size(), 0, (sockaddr*)&server_address, sizeof(server_address)) <= 0) {
        cout << "send / err :" << errno << endl;
        return false;
    }
    else {
        cout << "command has been issued" << endl;
    }

    return true;
}

bool AdminClient::get_response() {
    socklen_t server_address_len = sizeof(server_address);
    if (recvfrom(client_socket, _response.data(), _response.size(), 0, (sockaddr*)&server_address, &server_address_len) <= 0 ) {

        switch (errno) {
            case CONNECTION_REFUSED:
                cout << "receive / connection refused" << endl;
                break;
            case TIMEOUT:
                cout << "receive / timeout" << endl;
                break;
            default:
                cout << "receive / unknown error : " << errno << endl;
        }
        return false;
    }
    cout << "response from admin server :" << _response.data() << endl;
    return true;
}

int AdminClient::run() {
    init_socket(SOCK_DGRAM);

    bool contin = true;

    while (contin){
        contin = send_command();

        if (contin)
            contin = get_response();
    }

    return 0;
}