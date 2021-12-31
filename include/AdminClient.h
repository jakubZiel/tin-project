//
// Created by laura on 30.12.2021.
//

#ifndef TIN_21Z_ADMINCLIENT_H
#define TIN_21Z_ADMINCLIENT_H

#include "sockets.h"
#include <vector>

class AdminClient {
private:
    sockaddr_in server_address;

    struct timeval tv;

    int client_socket;

    std::vector<char> _response;
    std::vector<char> _command;

public:
    AdminClient();

    int init_socket(int protocol_type);

    sockaddr_in inet_association(sa_family_t in_family, in_port_t port, in_addr_t address);

    bool send_command();
    bool get_response();

    int run();

};

#endif //TIN_21Z_ADMINCLIENT_H
