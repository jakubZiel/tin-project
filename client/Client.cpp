#include <iostream>
#include <unistd.h>
#include <cstring>

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

    int counter = 0;
    while (true) {
        sleep(1);

        if (strcmp(client_mode.data(), "1") == 0) {
            if (sendto(client_socket, request_buffer.data(), request_buffer.size(), 0, (sockaddr *) &server_address,
                       sizeof(server_address)) <= 0) {
                cout << "Send / Err :" << errno << endl;
                break;
            } else {
                counter++;
                cout << "Wrote " << counter << endl;
            }
        }

        if (strcmp(client_mode.data(), "2") == 0) {
            socklen_t server_address_len = sizeof(server_address);
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(client_socket, &rfds);

            if (select(client_socket+1, &rfds, NULL, NULL, &tv)) {
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
                    }
                    break;
                }
                cout << "Message for :" << is_last.data() << endl;
            }
        }
    }

    cout << "Client :" << request_buffer.data() << " Sent :" << counter;
    return 0;
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
