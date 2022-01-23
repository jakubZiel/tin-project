//
// Created by jzielins on 23.01.2022.
//

#include "ChannelManager.h"

bool ChannelManager::is_banned(const std::string& client, const std::string& channel) {
    return channels[channel].banned.find(client) == channels[channel].banned.end();
}

bool ChannelManager::can_send(const std::string& client, const std::string& channel) {
    return channels[channel].senders.find(client) == channels[channel].senders.find(channel);
}

bool ChannelManager::can_listen(const std::string& client, const std::string& channel) {
    return !is_banned(client, channel) && channels[channel].listening_clients < channels[channel].max_size;
}

void ChannelManager::set_privacy(const std::string& channel, bool is_private) {
    channels[channel].is_private = is_private;
}

void ChannelManager::set_max_size(const std::string& channel, size_t size) {
    if (size > MAX_SIZE || size < 1)
        return;
    channels[channel].max_size = size;
}

void ChannelManager::ban_from_channel(const std::string& channel, const std::string& client) {
    channels[channel].banned[client] = ClientInfo();
}

std::string ChannelManager::get_clients(std::string channel) {
    return "clients : 1, 2, 3, 4, 5";
}