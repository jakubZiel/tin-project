//
// Created by jzielins on 23.01.2022.
//

#include "ChannelManager.h"

bool ChannelManager::is_banned(const std::string& client, const std::string& channel) {
    if (channels.find(channel) != channels.end())
        return channels[channel].banned.find(client) != channels[channel].banned.end();
    return false;
}

bool ChannelManager::can_send(const std::string &client, const std::string &channel, int current_clients) {
    if (channels.find(channel) != channels.end()) {
        return !is_banned(client, channel)
               && channels[channel].clients < channels[channel].max_size
               && current_clients < channels[channel].max_size;
    } else {
        channels[channel] = Channel();
        return true;
    }
}

bool ChannelManager::can_listen(const std::string& client, const std::string& channel, int current_clients) {
    if (channels.find(channel) != channels.end()) {
        channels[channel].clients = current_clients;
        return !is_banned(client, channel)
               && channels[channel].clients < channels[channel].max_size
               && current_clients < channels[channel].max_size;
    } else {
        channels[channel] = Channel();
        return true;
    }
}

void ChannelManager::set_privacy(const std::string& channel, bool is_private) {
    if (channels.find(channel) != channels.end())
        channels[channel].is_private = is_private;
}

void ChannelManager::set_max_size(const std::string& channel, size_t size) {
    if (size > MAX_SIZE || size < 1)
        return;
    if (channels.find(channel) != channels.end()) {
        channels[channel].max_size = size;
    } else {
        channels[channel] = Channel();
        channels[channel].max_size = size;
    }
}

void ChannelManager::ban_from_channel(const std::string& channel, const std::string& client) {
    if (channels.find(channel) != channels.end())
        channels[channel].banned[client] = ClientInfo();
}

void ChannelManager::unban_from_channel(const std::string &channel, const std::string &client) {
    if (channels.find(channel) != channels.end())
        channels[channel].banned.erase(client);
}

std::string ChannelManager::get_banned_users(const std::string& channel) {

    auto req_channel = channels[channel];
    std::string banned_users;

    for (auto banned : req_channel.banned){
        banned_users += banned.first + " | ";
    }

    return banned_users;
}