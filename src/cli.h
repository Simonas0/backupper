#ifndef CLI_H
#define CLI_H

#include "logger.h"

class Cli
{
    time_t getTime(char *str);

public:
    Cli(Logger *logger);
};

#endif
