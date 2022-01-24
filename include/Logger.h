//
// Created by jzielins on 24.01.2022.
//

#ifndef TIN_21Z_LOGGER_H
#define TIN_21Z_LOGGER_H

#include <string>

using path_type = std::string;

class Logger {
    std::string log_file;

public:
    Logger(std::string path);
    void log(std::string event);
};

#endif //TIN_21Z_LOGGER_H
