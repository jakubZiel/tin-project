//
// Created by jzielins on 23.01.2022.
//

#ifndef TIN_21Z_CHANNELMANAGER_H
#define TIN_21Z_CHANNELMANAGER_H

#include <unordered_map>
#include <string>
#define MAX_SIZE 100
#define DEFAULT_SIZE 15
class ClientInfo {
public:
    std::string id;
};

class Channel {
public:
    bool is_private;
    std::unordered_map<std::string, ClientInfo> banned;
    size_t max_size = DEFAULT_SIZE;
    size_t clients;
    std::unordered_map<std::string, ClientInfo> senders;
};

class ChannelManager {
    std::unordered_map<std::string, Channel> channels;
public:
    bool is_banned(const std::string& client, const std::string& channel);
    bool can_send(const std::string &client, const std::string &channel, int i);
    bool can_listen(const std::string& client, const std::string& channel, int current_clients);

    void set_privacy(const std::string& channel, bool is_private);
    void set_max_size(const std::string& channel, size_t size);
    void ban_from_channel(const std::string& channel, const std::string& client);
    void unban_from_channel(const std::string &channel, const std::string& client);

    std::string get_banned_users(const std::string& channel);
};
#endif
