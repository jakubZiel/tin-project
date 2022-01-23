//
// Created by jzielins on 23.01.2022.
//

#include "ChannelManager.h"

bool ChannelManager::is_banned(std::string client, std::string channel) {
    return channels[channel].banned.find(client) != channels[channel].banned.end();
}

bool ChannelManager::can_send(std::string client, std::string channel) {
    return channels[channel].sender == client;
}

bool ChannelManager::can_listen(std::string client, std::string channel) {
    return !is_banned(client, channel) && channels[channel].listening_clients < channels[channel].max_size;
}

void ChannelManager::set_privacy(std::string channel, bool is_private) {
    channels[channel].is_private = is_private;
}

void ChannelManager::set_max_size(std::string channel, size_t size) {
    if (size > MAX_SIZE || size < 1)
        return;

    channels[channel].max_size = size;
}

void ChannelManager::ban_from_channel(std::string channel, std::string client) {
    channels[channel].banned[client] = ClientInfo();
}