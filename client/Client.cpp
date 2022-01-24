#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <sys/signalfd.h>
#include <csignal>
#include <cstdlib>
#include <rapidjson/document.h>

#include "Client.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;
using namespace rapidjson;

Client::Client() {
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    server_address = inet_association(AF_INET, SERVER_PORT, inet_addr("127.0.0.1"));

    request_buffer = vector<char>(200);
    is_last = vector<char>(200);
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
    string message;
    while (true) {
        cout << "\nChannel: ";
        string channel;
        getline(cin, channel);
        if (channel == "end") {
            end_sending();
            break;
        }
        cout << "\nMessage: ";
        string message;
        getline(cin, message);

        affected_channels.insert(channel);
        prepare_message(channel, message, false);
    }
}

void Client::handle_batch_session() {
    cout << "File path: ";
    string path;
    getline(cin, path);
    ifstream message_file;
    message_file.open(path);

    string line;
    while (getline(message_file, line)) {
        vector<string> args;
        split(line, args, ' ');
        affected_channels.insert(args[0]);
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
    channel_listened_to = channel;

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
                    cout << "Received message:" << is_last.data() << endl;
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
    id_json = StringRef(to_string(client_socket).c_str());

    result.AddMember("channel", channel_json, result.GetAllocator());
    result.AddMember("listener", listener_json, result.GetAllocator());
    result.AddMember("message", message_json, result.GetAllocator());
    result.AddMember("userId", id_json, result.GetAllocator());

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    result.Accept(writer);

    send_data_to_server(buffer.GetString());
}

bool Client::send_data_to_server(const char * data) {
    strcpy(request_buffer.data(), data);
    if (sendto(client_socket, request_buffer.data(), request_buffer.size(), 0, (sockaddr *) &server_address, sizeof(server_address)) <= 0) {
        cout << "Send / Err :" << errno << endl;
        return false;
    } else {
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

    Document document;
    document.SetObject();

    Value channel;
    channel = END_MESSAGE;

    Value user_id;
    user_id = StringRef(to_string(client_socket).c_str());

    Value is_listener;
    is_listener = true;

    string channels;
    Value channels_payload;

    channels_payload = StringRef(channel_listened_to.c_str());

    document.AddMember("channel", channel,  document.GetAllocator());
    document.AddMember("message", channels_payload, document.GetAllocator());
    document.AddMember("userId", user_id, document.GetAllocator());
    document.AddMember("listener", is_listener, document.GetAllocator());

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    document.Accept(writer);
    string message = buffer.GetString();
    sendto(client_socket, &message[0], message.length(), 0, (sockaddr *) &server_address, sizeof(server_address));
    close(client_socket);

    client_active = false;
}

void Client::end_sending(){
    Document document;
    document.SetObject();

    Value channel;
    channel = END_MESSAGE;

    Value user_id;
    user_id = StringRef(to_string(client_socket).c_str());

    string channels;
    Value channels_payload;
    for (const auto & affected_channel : affected_channels){
        channels += affected_channel + "^";
    }
    channels_payload = StringRef(channels.c_str());

    Value is_listener;
    is_listener = false;

    document.AddMember("channel", channel,  document.GetAllocator());
    document.AddMember("message", channels_payload, document.GetAllocator());
    document.AddMember("userId", user_id, document.GetAllocator());
    document.AddMember("listener", is_listener, document.GetAllocator());


    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    document.Accept(writer);
    string message = buffer.GetString();
    sendto(client_socket, &message[0], message.length(), 0, (sockaddr *) &server_address, sizeof(server_address));
    close(client_socket);
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


int main(int argc, char** argv) {
    Client client;
    client.run();
    cout << "\nClosed gracefully...";
    return 0;
}
