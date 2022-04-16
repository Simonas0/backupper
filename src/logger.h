#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <mutex>
#include <regex>
#include <string>

#include "action.h"

#define ALTERED 0b00000001
#define BACKEDUP 0b00000010
#define CREATED 0b00000100
#define DELETED 0b00001000
#define REGEX 0b00010000
#define FROM 0b00100000
#define TO 0b01000000

class Logger
{
    std::ofstream logFile;
    std::mutex logFileMutex;

public:
    Logger();
    void log(std::string *path, Action action);
    void printLog(
        uint8_t mask, std::regex regex,
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to);
};

#endif
