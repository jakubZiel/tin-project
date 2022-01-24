//
// Created by jzielins on 24.01.2022.
//

#ifndef TIN_21Z_LOGGER_H
#define TIN_21Z_LOGGER_H

#include <string>
#include <fstream>
#include <rapidjson/document.h>

using path_type = std::string;

class Logger {
    std::ofstream file;
public:
    Logger() = default;
    Logger(std::string path);
    void log(rapidjson::Document & event);
    ~Logger();
};

#endif //TIN_21Z_LOGGER_H
