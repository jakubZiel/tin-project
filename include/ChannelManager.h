//
// Created by jzielins on 23.01.2022.
//

#ifndef TIN_21Z_CHANNELMANAGER_H
#define TIN_21Z_CHANNELMANAGER_H

#include <unordered_map>
#include <string>
#define MAX_SIZE 100

class ClientInfo {
public:
    std::string id;
};

class Channel {
public:
    bool is_private;
    std::unordered_map<std::string, ClientInfo> banned;
    size_t max_size;
    size_t listening_clients;
    std::string sender;
};


class ChannelManager {
    std::unordered_map<std::string, Channel> channels;
    bool is_banned(std::string client, std::string channel);
    bool can_send(std::string client, std::string channel);
    bool can_listen(std::string client, std::string channel);

    void set_privacy(std::string channel, bool is_private);
    void set_max_size(std::string channel, size_t size);
    void ban_from_channel(std::string channel, std::string client);
};
#endif
