//
// Created by jzielins on 24.01.2022.
//

#include "Logger.h"
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>


using namespace rapidjson;

Logger::Logger(std::string path) {
    file.open(path);
}

void Logger::log(Document& event) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    event.Accept(writer);

    file << buffer.GetString() << std::endl;
}

Logger::~Logger(){
    file.close();
}
