//
// Created by skons on 22.01.2022.
//

#include "Message.h"

using namespace rapidjson;

Message::Message(const char *message) {
    std::string message_string = message;
    Document message_json;
    ParseResult result = message_json.Parse(message);
    if (result) {
        assert(message_json.HasMember("channel"));
        assert(message_json["channel"].IsString());
        assert(message_json.HasMember("listener"));
        assert(message_json["listener"].IsBool());
        assert(message_json.HasMember("message"));
        assert(message_json["message"].IsString());
        assert(message_json.HasMember("userId"));
        assert(message_json["userId"].IsString());
        this->channel = message_json["channel"].GetString();
        this->is_listener = message_json["listener"].GetBool();
        this->message = message_json["message"].GetString();
        this->user_id = message_json["userId"].GetString();
    } else {
        throw std::invalid_argument("Invalid message - could not parse JSON.");
    }


}
