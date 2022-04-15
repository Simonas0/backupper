#include <iomanip>
#include <iostream>
#include <regex>

#include <string.h>

#include "cli.h"

#define DEFAULT_MASK ALTERED | BACKEDUP | CREATED | DELETED
#define BOLD "\e[1m"
#define NORMAL "\e[0m"

Cli::Cli(Logger *logger)
{
    uint8_t mask = DEFAULT_MASK;
    std::regex regex;
    std::string regexStr;
    char fromStr[32], toStr[32];
    time_t from, to;

    while (true)
    {
        // print logs
        (*logger).printLog(mask, regex, from, to);

        // prepare displayed variables
        if ((mask & FROM) == FROM)
        {
            strftime(fromStr, sizeof(fromStr), "%FT%TZ", std::gmtime(&from));
        }
        else
        {
            strcpy(fromStr, "time");
        }
        if ((mask & TO) == TO)
        {
            strftime(toStr, sizeof(toStr), "%FT%TZ", std::gmtime(&to));
        }
        else
        {
            strcpy(toStr, "time");
        }

        // print out commands
        std::cout << "Filters: " << std::endl
                  << ((mask & ALTERED) == ALTERED ? BOLD : NORMAL) << "[A]ltered" << NORMAL << " | "
                  << ((mask & BACKEDUP) == BACKEDUP ? BOLD : NORMAL) << "[B]acked-up" << NORMAL << " | "
                  << ((mask & CREATED) == CREATED ? BOLD : NORMAL) << "[C]reated" << NORMAL << " | "
                  << ((mask & DELETED) == DELETED ? BOLD : NORMAL) << "[D]eleted" << NORMAL << std::endl
                  << ((mask & REGEX) == REGEX ? BOLD : NORMAL) << "[R]egex " << regexStr << NORMAL << " | "
                  << ((mask & FROM) == FROM ? BOLD : NORMAL) << "[F]rom " << fromStr << NORMAL << " | "
                  << ((mask & TO) == TO ? BOLD : NORMAL) << "[T]o " << toStr << NORMAL << std::endl
                  << "Other:" << std::endl
                  << "Re[S]et filters | Re[L]oad results | [Q]uit" << std::endl;

        while (true)
        {
            // wait for input
            system("stty raw -echo");
            char c = getchar();
            system("stty -raw echo");

            // decide what to do
            switch (c)
            {
            case 'a':
                mask ^= ALTERED;
                break;
            case 'b':
                mask ^= BACKEDUP;
                break;
            case 'c':
                mask ^= CREATED;
                break;
            case 'd':
                mask ^= DELETED;
                break;
            case 'r':
                mask ^= REGEX;
                if ((mask & REGEX) == REGEX)
                {
                    while (true)
                    {
                        std::cout << "Enter regex: ";
                        std::cin >> regexStr;
                        try
                        {
                            regex = std::regex(regexStr);
                            break;
                        }
                        catch (const std::exception &e)
                        {
                            std::cout << "Regex could not be parsed" << std::endl;
                        }
                    }
                }
                else
                {
                    regexStr = "";
                }
                break;
            case 'f':
                mask ^= FROM;
                if ((mask & FROM) == FROM)
                {
                    while (true)
                    {
                        std::cout << "Enter from-time (UTC) in ISO format (2022-04-14T15:12:24Z): ";
                        std::cin >> fromStr;
                        from = getTime(fromStr);
                        if (from == -1)
                        {
                            std::cout << "Time could not be parsed" << std::endl;
                            continue;
                        }
                        break;
                    }
                }
                break;
            case 't':
                mask ^= TO;
                if ((mask & TO) == TO)
                {
                    while (true)
                    {
                        std::cout << "Enter to-time (UTC) in ISO format (2022-04-14T15:12:24Z): ";
                        std::cin >> toStr;
                        to = getTime(toStr);
                        if (to == -1)
                        {
                            std::cout << "Time could not be parsed" << std::endl;
                            continue;
                        }
                        break;
                    }
                }
                break;
            case 's':
                mask = DEFAULT_MASK;
                regexStr = "";
                strcpy(fromStr, "time");
                strcpy(toStr, "time");
                break;
            case 'l':
                break;
            case 'q':
                return;
            default:
                std::cout << "This key does nothing. Try another one" << std::endl;
                continue;
            }
            break;
        }
    }
}

time_t Cli::getTime(char *str)
{
    std::tm tm;
    if (strptime(str, "%FT%TZ", &tm) == 0)
    {
        return -1;
    }
    return mktime(&tm);
}