#ifndef TIN_21Z_CLIENT_H
#define TIN_21Z_CLIENT_H

#include <vector>
#include "sockets.h"

class Client {
private:
    struct timeval tv{};

    sockaddr_in server_address{};
    int client_socket{};

    std::vector<char> client_mode;
    std::vector<char> request_buffer;
    std::vector<char> is_last;

public:
    Client(char * request, char * mode);

    int init_socket(int protocol_type);
    sockaddr_in inet_association(sa_family_t in_family, in_port_t port, in_addr_t address);

    int run();

};
#endif TIN_21Z_CLIENT_H