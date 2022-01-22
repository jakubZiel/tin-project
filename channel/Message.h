//
// Created by skons on 22.01.2022.
//

#ifndef TIN_21Z_MESSAGE_H
#define TIN_21Z_MESSAGE_H

#include <string>

class Message {
    std::string message;
    std::string client_id;
public:
    Message(const char *message);

    std::string channel;
};


#endif //TIN_21Z_MESSAGE_H
