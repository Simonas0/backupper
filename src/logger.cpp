#include <iostream>

#include "debug.h"
#include "logger.h"
#include "time.h"

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

void Logger::printLog(uint8_t mask, std::regex regex,
                      std::chrono::system_clock::time_point from,
                      std::chrono::system_clock::time_point to)
{
    std::fstream logFile(FILE_NAME);
    std::string line;

    logFileMutex.lock();

    while (std::getline(logFile, line))
    {
        auto idx = line.find(' ');
        auto timeStr = line.substr(0, idx++);
        auto action = line.at(idx);
        auto name = line.substr(line.rfind('/') + 1);
        auto time = Time::stringToTime(&timeStr);

        switch (action)
        {
        case alter:
            if ((mask & ALTERED) != ALTERED)
            {
                continue;
            }
            break;
        case backup:
            if ((mask & BACKEDUP) != BACKEDUP)
            {
                continue;
            }
            break;
        case create:
            if ((mask & CREATED) != CREATED)
            {
                continue;
            }
            break;
        case delete_:
            if ((mask & DELETED) != DELETED)
            {
                continue;
            }
            break;
        default:
            continue;
        }

        if (((mask & REGEX) == REGEX && !std::regex_match(name, regex)) ||
            ((mask & FROM) == FROM && time < from) ||
            ((mask & TO) == TO && time > to))
        {
            continue;
        }

        std::cout << line << std::endl;
    }

    logFileMutex.unlock();
}
