//
// Created by skons on 22.01.2022.
//

#include "Message.h"

using namespace rapidjson;

Message::Message(const char *message) {
    std::string message_string = message;
    Document message_json;
    message_json.Parse(message);
    this->channel = message_json["channel"].GetString();
    this->message = message_json["message"].GetString();
    this->user_id = message_json["userId"].GetString();

}
