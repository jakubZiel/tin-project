#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <sys/signalfd.h>
#include <csignal>
#include <fcntl.h>
#include <cstdlib>
#include <random>
#include <rapidjson/document.h>

#include "Client.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;
using namespace rapidjson;

Client::Client() {
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    mt19937_64 mt(time(nullptr));
    client_id = mt() % 1000000 + 100000;

    server_address = inet_association(AF_INET, SERVER_PORT, inet_addr("127.0.0.1"));

    request_buffer = vector<char>(200);
    is_last = vector<char>(200);

    int is_banned;
}

int Client::run() {
    client_socket = init_socket(SOCK_DGRAM);
    client_active = true;

    prepare_signal_fd();

    string option = decide_input_method();
    if (option == "1") {
        handle_interactive_session();
    } else if (option == "2") {
        handle_batch_session();
    } else if (option == "3"){
        handle_listening_session();
    } else {
        cout << "No such option!" << endl;
    }
    return 0;
}

//1 - interactive
//2 - batch
//3 - listening
string Client::decide_input_method() {
    cout << "1 - interactive mode" << endl;
    cout << "2 - batch" << endl;
    cout << "3 - listening" << endl;
    cout << "Select input method: ";
    string option;
    getline(cin, option);
    return option;
}

void Client::handle_interactive_session() {
    //make_non_blocking(client_socket);

    string message;
    while (client_active) {
        cout << "\nChannel: ";
        string channel;
        getline(cin, channel);
        if (channel == "end") {
            break;
        }
        if (channel != "RESERVED_CHANNEL" && channel.find('^') == string::npos) {
            cout << "\nMessage: ";
            string message;
            getline(cin, message);
            prepare_message(channel, message, false);
        } else {
            cout << "\nForbidden channel name!";
        }
    }
}

void Client::handle_batch_session() {
    //make_non_blocking(client_socket);
    cout << "File path: ";
    string path;
    getline(cin, path);
    ifstream message_file;
    message_file.open(path);

    string line;
    while (getline(message_file, line)) {
        vector<string> args;
        split(line, args, ' ');
        prepare_message(args[0], args[1], false);
    }

    message_file.close();
}

void Client::handle_listening_session() {
    cout << "\nChannel: ";
    string channel;
    getline(cin, channel);
    string channel_setup = "setup";
    prepare_message(channel, channel_setup, true);

    while (client_active) {
        socklen_t server_address_len = sizeof(server_address);
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(client_socket, &rfds);
        FD_SET(signal_fd, &rfds);

        if (select(max(client_socket, signal_fd) + 1, &rfds, NULL, NULL, &tv)) {
            if (FD_ISSET(signal_fd, &rfds))
                handle_interrupt();
            else if (FD_ISSET(client_socket, &rfds)){
                if (recvfrom(client_socket, is_last.data(), is_last.size(), 0, (sockaddr *) &server_address,
                             &server_address_len) <= 0) {
                    switch (errno) {
                        case CONNECTION_REFUSED:
                            cout << "Receive / Connection refused" << endl;
                            break;
                        case TIMEOUT:
                            cout << "Receive / Timeout" << endl;
                            break;
                        default:
                            cout << "Receive / Unknown error" << endl;
                            break;
                    }
                } else {
                    cout << "Received message: " << is_last.data() << endl;
                }
            }
        }
    }
}

void Client::prepare_message(string &channel, string &message, bool is_listener) {
    Document result;
    result.SetObject();
    rapidjson::Value channel_json;
    channel_json = StringRef(channel.c_str());
    rapidjson::Value listener_json;
    listener_json = is_listener;
    rapidjson::Value message_json;
    message_json = StringRef(message.c_str());
    rapidjson::Value id_json;
    id_json = StringRef(to_string(client_id).c_str());

    result.AddMember("channel", channel_json, result.GetAllocator());
    result.AddMember("listener", listener_json, result.GetAllocator());
    result.AddMember("message", message_json, result.GetAllocator());
    result.AddMember("userId", id_json, result.GetAllocator());

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    result.Accept(writer);

    send_data_to_server(buffer.GetString(), message);
}

bool Client::send_data_to_server(const char * data, string message) {
    socklen_t server_address_len = sizeof(server_address);
    strcpy(request_buffer.data(), data);
    if (sendto(client_socket, request_buffer.data(), request_buffer.size(), 0, (sockaddr *) &server_address, sizeof(server_address)) <= 0) {
        cout << "Send / Err :" << errno << endl;
        return false;
    } else {
        is_banned = recvfrom(client_socket, is_last.data(), is_last.size(), 0, (sockaddr *) &server_address, &server_address_len);
        if (is_banned > 0 && is_last.data() != message) {
            cout << is_last.data() << endl;
        }
        cout << "Wrote " << data << endl;
        return true;
    }
}

size_t Client::split(const std::string &txt, std::vector<std::string> &strs, char ch) {
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while (pos != std::string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

    return strs.size();
}


int Client::init_socket(int protocol_type) {
    int n_socket = socket(AF_INET, protocol_type, 0);
    if (setsockopt(n_socket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error setting socket options");
    }
    return n_socket;
}

sockaddr_in Client::inet_association(sa_family_t in_family, in_port_t port, in_addr_t address) {
    sockaddr_in association{};
    association.sin_family = in_family;
    association.sin_port = htons(port);
    association.sin_addr.s_addr = address;
    return association;
}

void Client::handle_interrupt() {
    siginfo_t signal_info;
    read(signal_fd, &signal_info, sizeof(signal_info));
    cout << endl << "Interrupted by SIGINT" << endl;
    cout << signal_info.si_errno << endl;

    client_active = false;
}

void Client::prepare_signal_fd() {
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
}

void Client::make_non_blocking(int fd) {
    int status = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    if (status == -1){
        perror("error in fcntl");
        exit(errno);
    }
}


int main(int argc, char** argv) {
    Client client;
    client.run();
    cout << "\nClosed gracefully...";
    return 0;
}
