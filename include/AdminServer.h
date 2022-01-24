//
// Created by jzielins on 28.12.2021.
//

#ifndef TIN_21Z_ADMINSERVER_H
#define TIN_21Z_ADMINSERVER_H

#include "sockets.h"
#include <sys/signalfd.h>
#include <vector>
#include "ChannelManager.h"

#define BAN 1
#define USERS 2
#define SET_MAX_SIZE 3
#define SET_PRIVACY 4
#define UNBAN 5

#define BAD_COMMAND 100

class AdminServer {
private:
    sockaddr_in admin_server_address;
    sockaddr_in server_cmd_address;
    sockaddr_in admin_client_address;
    sockaddr_in msg_server_address;

    socklen_t msg_server_address_size;
    socklen_t admin_client_address_size;

    int admin_socket;
    int cmd_socket;
    int msg_server_connection_socket;

    fd_set server_sockets, ready_sockets;
    int signal_fd;

    int queue_size;
    std::vector<char> _request;
    std::vector<char> _response;
    std::vector<char> _command;
    std::vector<char> _command_response;

    bool admin_server_active;
    bool connection_opened;

    ChannelManager channelManager;

    std::unordered_map<std::string, int> command_table;

public:
    AdminServer();

    void handle_command(std::vector<char> &command, std::vector<char> &response);
    int bind_socket(int protocol_type, sockaddr_in& address_to_bind);
    void prepare_fdset();
    sockaddr_in inet_association(sa_family_t in_family, in_port_t port, in_addr_t address);
    void handle_msg_server_connection();
    void handle_msg_server_failed_connection();

    void handle_command_request();
    void handle_query();
    std::string handle_query(std::string &document);
    void handle_interrupt();

    void prepare_signal_fd();
    void make_non_blocking(int fd);
    void prepare_command_table();

    int run();
};
#endif //TIN_21Z_ADMINSERVER_H
