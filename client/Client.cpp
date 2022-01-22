#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fstream>

#include "Client.h"

using namespace std;

Client::Client(char * request, char * mode) {
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    server_address = inet_association(AF_INET, SERVER_PORT, inet_addr("127.0.0.1"));

    request_buffer = vector<char>(50);
    strcpy(request_buffer.data(), request);

    client_mode = vector<char>(50);
    strcpy(client_mode.data(), mode);

    is_last = vector<char>(50);
}

int Client::run() {

    client_socket = init_socket(SOCK_DGRAM);

    int option = decide_input_method();
    if (option == 1) {
        handle_interactive_session();
    } else if (option == 2) {
        handle_batch_session();
    } //else {
        //handle_listening_session();
    //}
    return 0;

//    int counter = 0;
//    while (true) {
//        sleep(1);
//
//        if (strcmp(client_mode.data(), "1") == 0) {
//            if (sendto(client_socket, request_buffer.data(), request_buffer.size(), 0, (sockaddr *) &server_address,
//                       sizeof(server_address)) <= 0) {
//                cout << "Send / Err :" << errno << endl;
//                break;
//            } else {
//                counter++;
//                cout << "Wrote " << counter << endl;
//            }
//        }
//
//        if (strcmp(client_mode.data(), "2") == 0) {
//            socklen_t server_address_len = sizeof(server_address);
//            fd_set rfds;
//            FD_ZERO(&rfds);
//            FD_SET(client_socket, &rfds);
//
//            if (select(client_socket+1, &rfds, NULL, NULL, &tv)) {
//                if (recvfrom(client_socket, is_last.data(), is_last.size(), 0, (sockaddr *) &server_address,
//                             &server_address_len) <= 0) {
//
//                    switch (errno) {
//                        case CONNECTION_REFUSED:
//                            cout << "Receive / Connection refused" << endl;
//                            break;
//                        case TIMEOUT:
//                            cout << "Receive / Timeout" << endl;
//                            break;
//                        default:
//                            cout << "Receive / Unknown error" << endl;
//                    }
//                    break;
//                }
//                cout << "Message for :" << is_last.data() << endl;
//            }
//        }
//    }
//
//    cout << "Client :" << request_buffer.data() << " Sent :" << counter;
//    return 0;
}


//1 - interactive
//2 - batch
//3 - listening
int Client::decide_input_method() {
    cout << "1 - interactive mode" << endl;
    cout << "2 - batch" << endl;
    cout << "3 - listening" << endl;
    cout << "Select input method: ";
    int option;
    cin >> option;
    return option;
}

void Client::handle_interactive_session() {
    string message;
    while (true) {
        cout << "\nChannel: ";
        string channel;
        cin >> channel;
        cout << "\nMessage: ";
        string message;
        cin >> message;
        if (message == "end") {
            break;
        }
        prepare_message(channel, message);
    }
}

void Client::handle_batch_session() {
    cout << "File path: ";
    string path;
    cin >> path;
    ifstream message_file;
    message_file.open(path);

    string line;
    while (getline(message_file, line)) {
        vector<string> args;
        split(line, args, ' ');
        prepare_message(args[0], args[1]);
    }

    message_file.close();
}

void Client::prepare_message(string &channel, string &message) {
    string data =
            "{\"channel\":" + channel + "\"message\":" + message + "\"usedId\":" + to_string(client_socket) + "}";
    send_data_to_server(data);
}

bool Client::send_data_to_server(string &data) {
    if (sendto(client_socket, request_buffer.data(), request_buffer.size(), 0, (sockaddr *) &server_address, sizeof(server_address)) <= 0) {
        cout << "Send / Err :" << errno << endl;
        return false;
    } else {
        cout << "Wrote " << data << endl;
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

int main(int argc, char** argv) {

    if (argc < 2) {
        cout << "Missing client number";
        return -2;
    }

    Client client(argv[1], argv[2]);
    client.run();
}
