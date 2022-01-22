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

    int client_socket;

    std::vector<char> _response;

    void send_data_to_server(std::string &data);

    int decide_input_method();

    void handle_interactive_session();
    void handle_command_arguments_interactive(int command_code);
    void handle_command_arguments_batch(int command_code, std::vector<std::string> &args);

    void handle_ban_user();
    void handle_max_users_on_channel();

    void prepare_ban_user_message(std::string &channel, std::string &client_id);
    void prepare_max_users_message(std::string &channel, std::string &max_users);

    void handle_batch_session();
    int parse_command(std::string &command);

    size_t split(const std::string &txt, std::vector<std::string> &strs, char ch);

    bool send_command(std::string &data);
    bool get_response();

public:
    AdminClient();



    int init_socket(int protocol_type);

    sockaddr_in inet_association(sa_family_t in_family, in_port_t port, in_addr_t address);


    int run();

};

#endif //TIN_21Z_ADMINCLIENT_H
