#ifndef TIN_21Z_SERVER_H
#define TIN_21Z_SERVER_H
#define HISTORY_SIZE 10
#define END_CHANNEL "RESERVED_CHANNEL"

#include "constants.h"
#include "../channel/Message.h"
#include "rapidjson/document.h"
#include "ClientInfo.h"
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "MessageHistory.h"

class Server {
public:
    Server();
    int run();
private:
    int server_socket{};
    int admin_socket{};
    fd_set sockets, ready_sockets;

    int signal_fd;
    bool server_active;

    sockaddr_in server_address{};
    sockaddr_in admin_server_address{};

    std::unordered_map<std::string, MessageHistory<Message, HISTORY_SIZE>> message_history;

    static sockaddr_in associate_inet(sa_family_t in_family, in_port_t port, in_addr_t address);
    std::vector<char> client_message;
    std::unordered_map<std::string, std::unordered_set<ClientInfo>> channels;
    std::vector<char> response;

    std::vector<char> admin_query;

    std::vector<char> admin_response;
    int connect_to_admin();
    static int bind_socket(int protocol_type, sockaddr_in& address_to_bind);

    size_t query_admin(std::string query);

    void prepare_fdset();

    void prepare_signal_fd();
    void handle_interrupt();
    const std::string create_admin_query(const Message& message);

    std::vector<std::string> find_client_channels(const std::string& channels_string, char delimiter);

    std::basic_string<char> print_message_history(std::string basicString);
};

#endif //TIN_21Z_SERVER_H
