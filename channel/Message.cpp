//
// Created by skons on 22.01.2022.
//

#include "Message.h"

Message::Message(const char *message) {
    std::string message_string = message;
    this->channel = message_string; // TODO real parsing

}
