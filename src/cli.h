#ifndef CLI_H
#define CLI_H

#include "logger.h"

class Cli
{
    uint8_t mask = 0b00000000;
    std::regex regex;
    std::string regexStr, fromStr, toStr;
    std::chrono::system_clock::time_point from, to;

    void saveState();
    void loadState();

public:
    Cli(Logger *logger);
};

#endif
