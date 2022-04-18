#include <iomanip>

#include "time.h"

#define ISO_TIME "%Y-%m-%dT%TZ"

std::chrono::system_clock::time_point Time::stringToTime(std::string const &str)
{
    tm tm;
    std::stringstream(str) >> std::get_time(&tm, ISO_TIME);
    const auto time = std::mktime(&tm);

    if (time == -1)
    {
        throw std::invalid_argument("\"" + str + "\" does not represent time in ISO format");
    }
    return std::chrono::system_clock::from_time_t(time);
}

std::string Time::timeToString(std::chrono::system_clock::time_point const &time)
{
    char buf[32];
    const auto time_ = std::chrono::system_clock::to_time_t(time);
    std::strftime(buf, sizeof(buf), ISO_TIME, gmtime(&time_));
    return std::string(buf);
}
