#include <iomanip>
#include <iostream>
#include <regex>

#include <string.h>

#include "cli.h"

#define FILE_NAME "filters"
#define DEFAULT_MASK ALTERED | BACKEDUP | CREATED | DELETED
#define BOLD "\e[1m"
#define NORMAL "\e[0m"

Cli::Cli(Logger *logger)
{
    loadState();

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
                saveState();
                return;
            default:
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
            strcpy(fromStr, line.substr(strlen("FROM") + 1).c_str());
            from = getTime(fromStr);
            if (from == -1)
            {
                mask |= FROM;
                strcpy(fromStr, "");
            }
        }
        else if (line.rfind("TO", 0) == 0)
        {
            mask |= TO;
            strcpy(toStr, line.substr(strlen("TO") + 1).c_str());
            to = getTime(toStr);
            if (to == -1)
            {
                mask |= TO;
                strcpy(toStr, "");
            }
        }
    }
}
