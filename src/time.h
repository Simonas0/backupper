#ifndef TIME_H
#define TIME_H

#include <chrono>

namespace Time
{
    std::chrono::system_clock::time_point stringToTime(std::string const &str, bool const localTime = false);
    std::string timeToString(std::chrono::system_clock::time_point const &time, bool const localTime = false);
};

#endif
