//
// Created by jakub on 11.10.2021.
//
#include <iostream>
#include <unistd.h>
#include <fstream>

#include "AdminClient.h"

using namespace std;

int main() {
    AdminClient adminClient;
    adminClient.run();
    return 0;
}

AdminClient::AdminClient() {
    server_address = inet_association(AF_INET, CMD_PORT, inet_addr("127.0.0.1"));
    _response = vector<char>(50);
}

int AdminClient::init_socket(int protocol_type) {
    client_socket = socket(AF_INET, protocol_type, 0);
}

sockaddr_in AdminClient::inet_association(sa_family_t in_family, in_port_t port, in_addr_t address) {
    sockaddr_in association{};
    association.sin_family = in_family;
    association.sin_port = htons(port);
    association.sin_addr.s_addr = address;

    return association;
}

bool AdminClient::send_command(string &data) {
    if (sendto(client_socket, data.data(), data.size(), 0, (sockaddr *) &server_address, sizeof(server_address)) <= 0) {
        cout << "send / err :" << errno << endl;
        return false;
    } else {
        cout << "command has been issued" << endl;
    }
    return true;
}

bool AdminClient::get_response() {
    socklen_t server_address_len = sizeof(server_address);
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
    init_socket(SOCK_DGRAM);

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
    }
}

void AdminClient::handle_ban_user() {
    cout << "\nchannel: ";
    string channel;
    cin >> channel;
    cout << "\nclient_id: ";
    string client_id;
    cin >> client_id;
    prepare_ban_user_message(channel, client_id);
}

void AdminClient::prepare_ban_user_message(string &channel, string &client_id) {
    string message =
            "{command:ban,channel:" + channel + ",client_id:" + client_id + "}";
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
            "{command:ban,channel:" + channel + ",client_id:" + max_users + "}";
    send_data_to_server(message);
}

void AdminClient::handle_command_arguments_batch(int command_code, std::vector<string> &args) {
    switch (command_code) {
        case 1:
            prepare_ban_user_message(args[1], args[2]);
            break;
        case 2:
            prepare_max_users_message(args[1], args[2]);
            break;
    }
}
