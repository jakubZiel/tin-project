//
// Created by jzielins on 28.12.2021.
//
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/signalfd.h>
#include <csignal>
#include <fcntl.h>
#include <rapidjson/document.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <iostream>
#include <iomanip>
#include <ctime>

#include "AdminServer.h"
#include "constants.h"
#include "Logger.h"

using namespace std;
using namespace rapidjson;

AdminServer::AdminServer() : logger(LOGFILE) {
    queue_size = 20;

    admin_server_address = inet_association(AF_INET, ADMIN_PORT, INADDR_ANY);
    server_cmd_address = inet_association(AF_INET, CMD_PORT, INADDR_ANY);

    msg_server_address_size = sizeof(msg_server_address);

    _request = vector<char>(1000);
    _response = vector<char>(1000);
    _command = vector<char>(1000);
    _command_response = vector<char>(1000);

    admin_server_active = true;
    connection_opened = false;

    string logfile_path = LOGFILE;

    prepare_command_table();
}

int AdminServer::run() {
    admin_socket = bind_socket(SOCK_STREAM, admin_server_address);
    cmd_socket = bind_socket(SOCK_DGRAM, server_cmd_address);

    if (listen(admin_socket, queue_size)){
        cout <<  "failed to listen to a port";
        return errno;
    } else
        cout << "listening to a port : " <<  ADMIN_PORT << endl;

    prepare_fdset();
    prepare_signal_fd();

    while (admin_server_active) {
        msg_server_connection_socket = accept(admin_socket, (sockaddr*) &msg_server_address, &msg_server_address_size);
        make_non_blocking(msg_server_connection_socket);

        if (msg_server_connection_socket != -1) {
            connection_opened = true;
            handle_msg_server_connection();
        } else {
            handle_msg_server_failed_connection();
        }
    }

    close(cmd_socket);
    close(admin_socket);
    return 0;
}

void AdminServer::handle_msg_server_connection() {

    FD_SET(msg_server_connection_socket, &server_sockets);

    cout << "connected from : " << inet_ntoa(msg_server_address.sin_addr) << endl;
    cout << "waiting for queries..." << endl;

    while (connection_opened){
        ready_sockets = server_sockets;
        //TODO  add fdset for writing to sockets
        if (select(FD_SETSIZE, &ready_sockets, nullptr, nullptr, nullptr) < 0){
            cout << "select fail, errno : " << errno << endl;
            connection_opened = false;
        } else if (FD_ISSET(signal_fd, &ready_sockets)) {
            handle_interrupt();
        } else {
            if (FD_ISSET(msg_server_connection_socket,  &ready_sockets))
                handle_query();
            if (FD_ISSET(cmd_socket, &ready_sockets))
                handle_command_request();
        }
    }
    FD_CLR(msg_server_connection_socket, &server_sockets);
    close(msg_server_connection_socket);
}

void AdminServer::handle_query() {
    long status = recv(msg_server_connection_socket, _request.data(), _request.size(), 0);

    string query = string(_request.data());
    string response = handle_query(query);
    strcpy(_response.data(), &response[0]);
    cout << "QUERY RESPONSE : " << _response.data() << endl;

    send(msg_server_connection_socket, _response.data(), _response.size(), 0);
}


void AdminServer::handle_command_request() {
    admin_client_address_size = sizeof (admin_client_address);
    recvfrom(cmd_socket, _command.data(), _command.size(), 0, (sockaddr*) &admin_client_address, &admin_client_address_size);

    handle_command(_command, _command_response);

    sendto(cmd_socket, _command_response.data(), _command_response.size(), 0, (sockaddr*) &admin_client_address, admin_client_address_size);
}

void AdminServer::handle_msg_server_failed_connection() {
    cout << "connection to msg_server failed. Errno : " << errno << endl;
    cout << "waiting for next msg_server connection\n";
}

int AdminServer::bind_socket(int protocol_type, sockaddr_in &address_to_bind) {
    int n_socket = socket(AF_INET, protocol_type, 0);

    if (bind(n_socket, (sockaddr *) &address_to_bind, sizeof(address_to_bind))) {
        cout << "failed to bind command socket" << endl;
        return -1;
    } else
        cout << "command socket is bound" << endl;
    return n_socket;
}

void AdminServer::prepare_fdset() {
    FD_ZERO(&server_sockets);
    FD_SET(cmd_socket, &server_sockets);
}

sockaddr_in AdminServer::inet_association(sa_family_t in_family, in_port_t port, in_addr_t address) {
    sockaddr_in association{};
    association.sin_family = in_family;
    association.sin_port = htons(port);
    association.sin_addr.s_addr = address;
    return association;
}

void AdminServer::handle_command(std::vector<char> &request, std::vector<char> &response) {
    Document document;

    document.Parse(request.data());
    cout << request.data();
    string channel = document["channel"].GetString();
    string user;
    int size;

    switch (command_table[document["command"].GetString()]) {
        case BAN:
            user = document["user_id"].GetString();
            channelManager.ban_from_channel(channel, user);
            break;
        case UNBAN:
            user = document["user_id"].GetString();
            channelManager.unban_from_channel(channel, user);
            break;
        case USERS:
            channelManager.get_banned_users(channel);
            break;
        case SET_MAX_SIZE:
            size = document["max_users"].GetInt();
            channelManager.set_max_size(channel, size);
            break;
        case SET_PRIVACY:
            channelManager.set_privacy(channel, true);
            break;
        default:
            perror("bad command\n");
    }

    cout << "command handled : " << request.data() << endl;
    strcpy(response.data(), "HANDLED");
}

void AdminServer::prepare_signal_fd() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
        perror("can't block SIGINT for this process");
        exit(errno);
    }

    signal_fd = signalfd(-1, &mask, 0);

    if (signal_fd == -1){
        perror("can't create signal fd");
        exit(errno);
    }

    FD_SET(signal_fd, &server_sockets);
}

void AdminServer::handle_interrupt() {
    siginfo_t signal_info;
    read(signal_fd, &signal_info, sizeof(signal_info));
    cout << endl << "Interrupted by SIGINT" << endl;
    cout << signal_info.si_errno << endl;
    connection_opened = false;
    admin_server_active = false;
}

void AdminServer::make_non_blocking(int fd) {
    int status = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    if (status == -1){
        perror("error in fcntl");
        exit(errno);
    }
}

string AdminServer::handle_query(string& query){
    Document document;
    StringStream stream(query.c_str());

    document.ParseStream<kParseStopWhenDoneFlag>(stream);

    bool is_authorized;

    if (document["listener"].GetBool()){
        is_authorized = channelManager.can_listen(document["userId"].GetString(), document["channel"].GetString(), document["current_users_number"].GetInt());
    } else {
        is_authorized = channelManager.can_send(document["userId"].GetString(), document["channel"].GetString());
    }

    if (!is_authorized) {
        Value date;
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
        auto str = oss.str();
        date = StringRef(str.c_str());

        document.AddMember("timestamp", date, document.GetAllocator());
        logger.log(document);
    }

    Document response;
    response.SetObject();
    response.AddMember("authorized", is_authorized, response.GetAllocator());

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    response.Accept(writer);
    return buffer.GetString();
}

void AdminServer::prepare_command_table() {
    command_table["ban"] = BAN;
    command_table["unban"] = UNBAN;
    command_table["get_banned_users"] = USERS;
    command_table["set_max_users"] = SET_MAX_SIZE;
    command_table["set_privacy"] = SET_PRIVACY;
}

int main(){
    AdminServer server;
    server.run();
    cout << "\nClosed gracefully...";
    return 0;
}
