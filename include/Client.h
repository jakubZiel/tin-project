#ifndef TIN_21Z_CLIENT_H
#define TIN_21Z_CLIENT_H

#include <vector>
#include "sockets.h"

class Client {
private:
    struct timeval tv{};

    sockaddr_in server_address{};
    int client_socket{};

    std::vector<char> request_buffer;
    std::vector<char> is_last;

    int decide_input_method();

    void handle_interactive_session();
    void handle_batch_session();

    void prepare_message(std::string &channel, std::string &message);
    bool send_data_to_server(std::string &data);

    size_t split(const std::string &txt, std::vector<std::string> &strs, char ch);


public:
    Client();

    int init_socket(int protocol_type);
    sockaddr_in inet_association(sa_family_t in_family, in_port_t port, in_addr_t address);

    int run();

};
#endif TIN_21Z_CLIENT_H