#include <ctime>
#include <mutex>

#include "debug.h"
#include "logger.h"

#define FILE_NAME "log"

Logger::Logger()
{
    logFile.open(FILE_NAME, std::ofstream::app);
}

void Logger::log(std::string *path, Action action)
{
    auto time = std::time(nullptr);
    char buf[32];

    strftime(buf, sizeof(buf), "%FT%TZ", std::gmtime(&time));

    logFileMutex.lock();
    logFile << buf << ' ' << (char)action << ' ' << *path << std::endl;
    logFileMutex.unlock();
}
