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
    void handle_get_users();
    void handle_set_channel_mode();
    void handle_set_max_stored_messages();
    void handle_unban_user();

    void prepare_ban_user_message(std::string &channel, std::string &user_id);
    void prepare_max_users_message(std::string &channel, std::string &max_users);
    void prepare_get_users_message(std::string &channel);
    void prepare_set_channel_mode_message(std::string &channel, std::string &mode);
    void prepare_set_max_stored_messages(std::string &channel, std::string &max_stored_messages);
    void prepare_unban_user_message(std::string &channel, std::string &user_id);

    void handle_batch_session();
    int parse_command(std::string &command);

    size_t split(const std::string &txt, std::vector<std::string> &strs, char ch);

    bool send_command(std::string &data);
    bool get_response();

public:
    AdminClient();
    int run();
};

#endif //TIN_21Z_ADMINCLIENT_H
