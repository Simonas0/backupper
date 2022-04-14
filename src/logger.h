#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

#include "action.h"

class Logger
{
    std::ofstream logFile;
    std::mutex logFileMutex;

public:
    Logger();
    void log(std::string *path, Action action);
};

#endif
