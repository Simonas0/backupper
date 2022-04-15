#ifndef CLI_H
#define CLI_H

#include "logger.h"

class Cli
{
    uint8_t mask = 0b00000000;
    std::regex regex;
    std::string regexStr;
    time_t from, to;
    char fromStr[32], toStr[32];

    time_t getTime(char *str);
    void saveState();
    void loadState();

public:
    Cli(Logger *logger);
};

#endif
