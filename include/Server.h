#ifndef TIN_21Z_SERVER_H
#define TIN_21Z_SERVER_H
#define NUMBER_OF_CONNECTIONS 100

#include "sockets.h"
#include "../channel/Message.h"
#include "rapidjson/document.h"
#include "ClientInfo.h"
#include <vector>
#include <unordered_set>
#include <unordered_map>

class Server {
public:
    Server();
    int run();
private:
    int server_socket{};
    int admin_socket{};
    fd_set sockets, ready_sockets;

    sockaddr_in server_address{};
    sockaddr_in admin_server_address{};
    static sockaddr_in associate_inet(sa_family_t in_family, in_port_t port, in_addr_t address);

    std::vector<char> client_id;
    std::unordered_set<ClientInfo> clients; // TODO remove?
    std::unordered_map<std::string, std::unordered_set<ClientInfo>> channels;
    std::vector<char> response;
    std::map<std::string, int> clients_datagram_count;

    std::vector<char> admin_query;
    std::vector<char> admin_response;

    int connect_to_admin();
    static int bind_socket(int protocol_type, sockaddr_in& address_to_bind);
    size_t query_admin(char* query);

    bool server_active;

    void prepare_fdset();

};

#endif //TIN_21Z_SERVER_H
