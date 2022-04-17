#include <iostream>

#include "debug.h"
#include "logger.h"
#include "time.h"

#define FILE_NAME "log"

Logger::Logger()
{
    logFile.open(FILE_NAME, std::ofstream::app);
}

void Logger::log(std::string const &path, const Action action)
{

    logFileMutex.lock();
    logFile << Time::timeToString(std::chrono::system_clock::now())
            << ' ' << (char)action << ' ' << path << std::endl;
    logFileMutex.unlock();
}

void Logger::printLog(uint8_t mask, std::regex const &regex,
                      std::chrono::system_clock::time_point const &from,
                      std::chrono::system_clock::time_point const &to)
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
        std::chrono::system_clock::time_point time;

        try
        {
            time = Time::stringToTime(timeStr);
        }
        catch (std::exception &e)
        {
            std::cerr << "Cannot parse time from line: " << line << std::endl;
            continue;
        }

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
            std::cerr << "Cannot parse action from line: " << line << std::endl;
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
