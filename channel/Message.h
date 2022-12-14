//
// Created by skons on 22.01.2022.
//

#ifndef TIN_21Z_MESSAGE_H
#define TIN_21Z_MESSAGE_H

#include <string>
#include "rapidjson/document.h"

class Message {
public:
    std::string channel;
    bool is_listener;
    std::string message;
    std::string user_id;
    Message() = default;
    explicit Message(const char *message);
};


#endif //TIN_21Z_MESSAGE_H
