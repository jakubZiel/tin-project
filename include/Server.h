#ifndef TIN_21Z_SERVER_H
#define TIN_21Z_SERVER_H
#define NUMBER_OF_CONNECTIONS 100

#include "sockets.h"
#include <vector>
class Server {
public:
    Server();
    int run();
private:
    int number_of_connections = NUMBER_OF_CONNECTIONS; // TODO remove in the future, for development purposes
    int server_socket{};
    int admin_socket{};

    sockaddr_in server_address{};
    sockaddr_in client_address{};
    sockaddr_in admin_server_address{};
    static sockaddr_in associate_inet(sa_family_t in_family, in_port_t port, in_addr_t address);

    std::vector<char> client_id;
    std::vector<char> response;
    std::map<std::string, int> clients_datagram_count;

    std::vector<char> admin_query;
    std::vector<char> admin_response;

    int connect_to_admin();
    static int bind_socket(int protocol_type, sockaddr_in& address_to_bind);
    size_t query_admin(char* query);

};
#endif //TIN_21Z_SERVER_H
