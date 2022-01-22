//
// Created by jzielins on 28.12.2021.
//

#ifndef TIN_21Z_ADMINSERVER_H
#define TIN_21Z_ADMINSERVER_H

#include "sockets.h"
#include <sys/signalfd.h>
#include <vector>

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

public:
    AdminServer();

    void find_record(std::vector<char> &request, std::vector<char> &response);
    void handle_command(std::vector<char> &command, std::vector<char> &response);
    int bind_socket(int protocol_type, sockaddr_in& address_to_bind);
    void prepare_fdset();
    sockaddr_in inet_association(sa_family_t in_family, in_port_t port, in_addr_t address);
    void handle_msg_server_connection();
    void handle_msg_server_failed_connection();

    void handle_command_request();
    void handle_query();
    void handle_interrupt();

    void prepare_signal_fd();

    int run();

};
#endif //TIN_21Z_ADMINSERVER_H
