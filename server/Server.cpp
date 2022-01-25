#include <iostream>
#include <unistd.h>
#include <cstring>
#include <map>
#include <sys/signalfd.h>
#include <csignal>

#include "constants.h"
#include "Server.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"


using namespace std;
using namespace rapidjson;

Server::Server() {

    server_address = associate_inet(AF_INET, SERVER_PORT, INADDR_ANY);
    admin_server_address = associate_inet(AF_INET, ADMIN_PORT, inet_addr("127.0.0.1"));

    client_message = vector<char>(2000);
    response = vector<char>(2000);
    admin_query = vector<char>(1000);
    admin_response = vector<char>(1000);

    server_active = true;

}

sockaddr_in Server::associate_inet(sa_family_t in_family, in_port_t port, in_addr_t address) {
    sockaddr_in association{};
    association.sin_family = in_family;
    association.sin_port = htons(port);
    association.sin_addr.s_addr = address;
    return association;
}

int Server::connect_to_admin() {
    admin_socket = socket(AF_INET, SOCK_STREAM, 0);

    int connection_status = connect(admin_socket, (sockaddr*) &admin_server_address, sizeof(admin_server_address));
    if (connection_status == -1){
        cout << "Failed to connect to admin server, errno: " << errno << endl;
        return errno;
    }
    cout << "Connected to admin server." << endl;
    return admin_socket;
}

int Server::bind_socket(int protocol_type, sockaddr_in &address_to_bind) {
    int n_socket = socket(AF_INET, protocol_type, 0);

    if (bind(n_socket, (sockaddr *) &address_to_bind, sizeof(address_to_bind))) {
        cout << "Failed to bind socket, errno: " << errno;
        return -1;
    } else
        cout << "Socket is bound." << endl;
    return n_socket;
}

size_t Server::query_admin(string query){
    int request_size = send(admin_socket, &query[0], query.size(), 0);
    long status = 0;

    if (request_size != -1) {
        status = recv(admin_socket, admin_response.data(), admin_response.size(), 0);

        if (status == 0) {
            server_active = false;
        }

    }  else {
        server_active = false;
    }
    return status;
}

int Server::run() {

    admin_socket = connect_to_admin();

    server_socket = bind_socket(SOCK_DGRAM, server_address);

    sockaddr_in client{};
    socklen_t socklen = sizeof(client);

    prepare_fdset();
    prepare_signal_fd();

    while (server_active) {
        ready_sockets = sockets;
        if (select(FD_SETSIZE, &ready_sockets, nullptr, nullptr, nullptr) < 0) {
            cout << "Select fail, errno: " << errno << endl;
            exit(errno);

        } else if (FD_ISSET(signal_fd, &ready_sockets)){
          handle_interrupt();

        } else if (FD_ISSET(server_socket, &ready_sockets)) {
            recvfrom(server_socket, client_message.data(), client_message.size(), 0, (sockaddr *) &client, &socklen);
            auto clientInfo =  ClientInfo(client);

            cout << "Datagram from client: " << client_message.data() << endl;
            Message message;
            try {
                message = Message(client_message.data());
            } catch (std::invalid_argument &e) {
                cerr << e.what() << endl;
                continue;
            }

            send_query_to_admin(message);

            if (server_active) {
                Document admin_response_json;
                admin_response_json.Parse(admin_response.data());
                if (!admin_response_json["authorized"].GetBool()) {
                    send_error_message(clientInfo);
                    channels[message.channel].erase(clientInfo);
                    continue;
                }

                if (message.channel == END_CHANNEL) {
                    remove_client_from_channels(clientInfo, message);
                    continue;
                }
                channels[message.channel].insert(clientInfo);

                if (!message.is_listener) {
                    send_message_to_clients(message);
                } else {
                    send_message_to_listener(clientInfo, message);
                }
            }
        }
    }
    FD_CLR(server_socket, &sockets);
    close(server_socket);

    return 0;
}

void Server::send_message_to_listener(const ClientInfo &clientInfo, Message &message) {
    strcpy(response.data(), print_message_history(message.channel).c_str());
    sendto(server_socket, response.data(), response.size(), 0, (sockaddr *) &clientInfo.addr, sizeof(clientInfo.addr));
}

void Server::send_message_to_clients(const Message &message) {
    strcpy(response.data(), message.message.c_str());
    message_history[message.channel].push(message);
    for (auto& cl : channels[message.channel]) {
        sendto(server_socket, response.data(), response.size(), 0, (sockaddr *) &cl.addr, sizeof(cl.addr));
    }
}

void Server::remove_client_from_channels(const ClientInfo &clientInfo, const Message &message) {
    auto client_channels = find_client_channels(message.message, '^');
    for (auto& channel : client_channels) {
        channels[channel].erase(clientInfo);
    }
}

void Server::send_query_to_admin(const Message &message) {
    string query = create_admin_query(message);
    strcpy(admin_query.data(), &query[0]);
    query_admin(admin_query.data());
}

void Server::send_error_message(const ClientInfo &clientInfo) {
    auto error_message =  "Channel access denied. You have either been banned or the channel is full.";
    cerr << error_message << endl;
    strcpy(response.data(), error_message);
    sendto(server_socket, response.data(), response.size(), 0, (sockaddr *) &clientInfo.addr, sizeof(clientInfo.addr));
}


void Server::prepare_fdset() {
    FD_ZERO(&sockets);
    FD_SET(server_socket, &sockets);
}

void Server::prepare_signal_fd() {
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

    FD_SET(signal_fd, &sockets);
}

void Server::handle_interrupt() {
    siginfo_t signal_info;
    read(signal_fd, &signal_info, sizeof(signal_info));
    cout << endl << "Interrupted by SIGINT" << endl;
    cout << signal_info.si_errno << endl;

    server_active = false;
}

string Server::create_admin_query(const Message& message) {
    Document query_json;
    query_json.SetObject();
    rapidjson::Value channel;
    channel = StringRef(message.channel.c_str());
    rapidjson::Value userId;
    userId = StringRef(message.user_id.c_str());
    rapidjson::Value listener(message.is_listener);
    rapidjson::Value current_users_number(channels[message.channel].size());

    query_json.AddMember("channel", channel, query_json.GetAllocator());
    query_json.AddMember("listener", listener, query_json.GetAllocator());
    query_json.AddMember("current_users_number", current_users_number, query_json.GetAllocator());
    query_json.AddMember("userId", userId, query_json.GetAllocator());
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    query_json.Accept(writer);
    string res = buffer.GetString();

    return buffer.GetString();
}

vector<string> Server::find_client_channels(const string& channels_string, char delimiter) {
    vector<string> result_channels;
    string current_channel;
    for (auto ch : channels_string) {
        if (ch == delimiter) {
            result_channels.push_back(current_channel);
            current_channel = "";
        } else {
            current_channel += ch;
        }
    }
    result_channels.push_back(current_channel);
    return result_channels;
}

string Server::print_message_history(std::string channel) {
    string result = "\nLast messages:\n";
    auto helper = MessageHistory<Message, HISTORY_SIZE>(message_history[channel]);
    while(!helper.empty()) {
        result += (helper.front().message) + "\n";
        helper.pop();
    }
    return result;
}

int main() {
    Server server;
    server.run();
    cout << "\nClosed gracefully...";
    return 0;
}

