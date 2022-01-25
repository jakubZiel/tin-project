//
// Created by jakub on 11.10.2021.
//
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cstring>

#include "AdminClient.h"

using namespace std;

int main() {
    AdminClient adminClient;
    adminClient.run();
    return 0;
}

AdminClient::AdminClient() {
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        cout << "ERROR - couldn't create socket" << endl;
        exit(1);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(CMD_PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    _response = vector<char>(500);
}

bool AdminClient::send_command(string &data) {
    cout <<  "Sending...\n";

    if (sendto(client_socket, data.c_str(), data.length(), 0, (sockaddr*)&server_address, sizeof(server_address)) <= 0) {
        cout << "send / err :" << errno << endl;
        return false;
    }
    else {
        cout << "command has been issued" << endl;
    }
    return true;
}

bool AdminClient::get_response() {
    socklen_t server_address_len = sizeof(server_address);
    memset(_response.data(), 0, _response.size());

    if (recvfrom(client_socket, _response.data(), _response.size(), 0, (sockaddr *) &server_address,
                 &server_address_len) <= 0) {

        switch (errno) {
            case CONNECTION_REFUSED:
                cout << "receive / connection refused" << endl;
                break;
            case TIMEOUT:
                cout << "receive / timeout" << endl;
                break;
            default:
                cout << "receive / unknown error : " << errno << endl;
        }
        return false;
    }
    cout << "response from admin server :" << _response.data() << endl;
    return true;
}

int AdminClient::run() {
    int option = decide_input_method();
    if (option == 1) {
        handle_interactive_session();
    } else {
        handle_batch_session();
    }
    return 0;
}

void AdminClient::send_data_to_server(string &data) {
    cout << "Data for server: " + data << endl;
    send_command(data);
}

//1 - interactive
//2 - batch
int AdminClient::decide_input_method() {
    cout << "1 - interactive mode" << endl;
    cout << "2 - batch" << endl;
    cout << "Select input method: ";
    int option;
    cin >> option;

    while (option != 1 && option != 2) {
        cout << "Wrong command - try again\n";
        cin >> option;
    }

    return option;
}

void AdminClient::handle_interactive_session() {
    string command;
    while (true) {
        cout << "\nCommand: ";
        cin >> command;
        if (command == "end") {
            break;
        }
        int command_code = parse_command(command);
        if (command_code == -1) {
            cout << "Wrong command - try again";
            continue;
        }
        handle_command_arguments_interactive(command_code);
    }
}

void AdminClient::handle_batch_session() {
    cout << "path: ";
    string path;
    cin >> path;
    ifstream file_with_commands;
    file_with_commands.open(path);

    string line;
    while (getline(file_with_commands, line)) {
        vector<string> args;
        split(line, args, ' ');
        int command_code = parse_command(args[0]);
        if (command_code == -1) {
            cout << "Wrong command - try again";
            continue;
        }
        handle_command_arguments_batch(command_code, args);
    }
    file_with_commands.close();
}

int AdminClient::parse_command(string &command) {
    if (command == "ban") {
        return 1;
    }
    if (command == "set_max_users") {
        return 2;
    }
    if (command == "get_banned_users") {
        return 3;
    }
    if (command == "set_max_stored_messages"){
        return 4;
    }
    if (command == "unban"){
        return 5;
    }

    //error code
    return -1;
}

size_t AdminClient::split(const std::string &txt, std::vector<std::string> &strs, char ch) {
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

void AdminClient::handle_command_arguments_interactive(int command_code) {
    switch (command_code) {
        case 1:
            handle_ban_user();
            break;
        case 2:
            handle_max_users_on_channel();
            break;
        case 3:
            handle_get_banned_users();
            break;
        case 4:
            handle_set_max_stored_messages();
            break;
        case 5:
            handle_unban_user();
            break;
    }
    get_response();

}

void AdminClient::handle_command_arguments_batch(int command_code, std::vector<string> &args) {
    switch (command_code) {
        case 1:
            prepare_ban_user_message(args[1], args[2]);
            break;
        case 2:
            prepare_max_users_message(args[1], args[2]);
            break;
        case 3:
            prepare_get_banned_users_message(args[1]);
            break;
        case 4:
            prepare_set_max_stored_messages(args[1]);
            break;
        case 5:
            prepare_unban_user_message(args[1], args[2]);
            break;
    }
}

void AdminClient::handle_ban_user() {
    cout << "\nchannel: ";
    string channel;
    cin >> channel;
    cout << "\nuser_id: ";
    string user_id;
    cin >> user_id;
    prepare_ban_user_message(channel, user_id);
}

void AdminClient::prepare_ban_user_message(string &channel, string &user_id) {
    string message =
            R"({"command":"ban","channel":")" + channel + R"(","user_id":")" + user_id + "\"}";
    send_data_to_server(message);
}

void AdminClient::handle_max_users_on_channel() {
    cout << "\nchannel: ";
    string channel;
    cin >> channel;
    cout << "\nmax users: ";
    string max_users;
    cin >> max_users;
    prepare_max_users_message(channel, max_users);
}

void AdminClient::prepare_max_users_message(string &channel, string &max_users) {
    string message =
            R"({"command":"max_users","channel":")" + channel + R"(","max_users":)" + max_users + "}";
    send_data_to_server(message);
}

void AdminClient::handle_get_banned_users() {
    cout << "\nchannel: ";
    string channel;
    cin >> channel;

    prepare_get_banned_users_message(channel);
}

void AdminClient::prepare_get_banned_users_message(string &channel) {
    string message =
            R"({"command":"get_banned_users","channel":")" + channel + "\"}";
    send_data_to_server(message);
}

void AdminClient::handle_set_max_stored_messages() {
    cout << "\nmax stored messages: ";
    string max_stored;
    cin >> max_stored;
    prepare_set_max_stored_messages(max_stored);
}

void AdminClient::prepare_set_max_stored_messages(string &max_stored_messages) {
    string message =
            R"({"command":"set_max_stored_messages","max_stored":)" + max_stored_messages + "\"}";
    send_data_to_server(message);
}

void AdminClient::handle_unban_user() {
    cout << "\nchannel: ";
    string channel;
    cin >> channel;
    cout << "\nuser_id: ";
    string user_id;
    cin >> user_id;
    prepare_unban_user_message(channel, user_id);
}

void AdminClient::prepare_unban_user_message(string &channel, string &user_id) {
    string message =
            R"({"command":"unban","channel":")" + channel + R"(","user_id":")" + user_id + "\"}";
    send_data_to_server(message);
}