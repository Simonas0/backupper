#include <iomanip>
#include <iostream>
#include <regex>

#include <string.h>

#include "cli.h"
#include "time.h"

#define FILE_NAME "filters"
#define DEFAULT_MASK ALTERED | BACKEDUP | CREATED | DELETED
#define BOLD "\e[1m"
#define NORMAL "\e[0m"

Cli::Cli(Logger &logger)
{
    loadState();

    while (true)
    {
        // print logs
        logger.printLog(mask, regex, from, to);

        // prepare displayed variables
        if ((mask & FROM) == FROM)
        {
            fromStr = Time::timeToString(from);
        }
        else
        {
            fromStr = "time";
        }
        if ((mask & TO) == TO)
        {
            toStr = Time::timeToString(to);
        }
        else
        {
            toStr = "time";
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
            const char c = getchar();
            system("stty -raw echo");

            // decide what to do
            switch (tolower(c))
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
                        std::cout << "Enter to-time (UTC) in ISO format (2022-04-14T15:12:24Z): ";
                        std::cin >> fromStr;
                        try
                        {
                            from = Time::stringToTime(fromStr);
                        }
                        catch (const std::exception &e)
                        {
                            std::cout << e.what() << std::endl;
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
                        try
                        {
                            to = Time::stringToTime(toStr);
                        }
                        catch (const std::exception &e)
                        {
                            std::cout << e.what() << std::endl;
                            continue;
                        }
                        break;
                    }
                }
                break;
            case 's':
                mask = DEFAULT_MASK;
                regexStr = "";
                fromStr = toStr = "time";
                break;
            case 'l':
                break;
            case 'q':
                saveState();
                return;
            default:
                continue;
            }
            break;
        }
    }
}

void Cli::saveState()
{
    std::ofstream file(FILE_NAME);

    if ((mask & ALTERED) == ALTERED)
    {
        file << "ALTERED" << std::endl;
    }
    if ((mask & BACKEDUP) == BACKEDUP)
    {
        file << "BACKEDUP" << std::endl;
    }
    if ((mask & CREATED) == CREATED)
    {
        file << "CREATED" << std::endl;
    }
    if ((mask & DELETED) == DELETED)
    {
        file << "DELETED" << std::endl;
    }
    if ((mask & REGEX) == REGEX)
    {
        file << "REGEX " << regexStr << std::endl;
    }
    if ((mask & FROM) == FROM)
    {
        file << "FROM " << fromStr << std::endl;
    }
    if ((mask & TO) == TO)
    {
        file << "TO " << toStr << std::endl;
    }
}

void Cli::loadState()
{
    std::ifstream file(FILE_NAME);

    std::string line;

    while (std::getline(file, line))
    {
        if (line.rfind("ALTERED", 0) == 0)
        {
            mask |= ALTERED;
        }
        else if (line.rfind("BACKEDUP", 0) == 0)
        {
            mask |= BACKEDUP;
        }
        else if (line.rfind("CREATED", 0) == 0)
        {
            mask |= CREATED;
        }
        else if (line.rfind("DELETED", 0) == 0)
        {
            mask |= DELETED;
        }
        else if (line.rfind("REGEX", 0) == 0)
        {
            mask |= REGEX;
            regexStr = line.substr(strlen("REGEX") + 1);
            try
            {
                regex = std::regex(regexStr);
            }
            catch (const std::exception &e)
            {
                mask |= REGEX;
                regexStr = "";
            }
        }
        else if (line.rfind("FROM", 0) == 0)
        {
            mask |= FROM;
            fromStr = line.substr(strlen("FROM") + 1);
            try
            {
                from = Time::stringToTime(fromStr);
            }
            catch (const std::exception &e)
            {
                mask |= FROM;
                fromStr = "time";
            }
        }
        else if (line.rfind("TO", 0) == 0)
        {
            mask |= TO;
            toStr = line.substr(strlen("TO") + 1);
            try
            {
                to = Time::stringToTime(toStr);
            }
            catch (const std::exception &e)
            {
                mask |= TO;
                toStr = "time";
            }
        }
    }
}
