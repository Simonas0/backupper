#ifndef BACKUPPER_H
#define BACKUPPER_H

#include <filesystem>
#include <future>
#include <map>
#include <vector>

#include "logger.h"

class Backupper
{
	Logger &logger;

	std::string const &hotDir;
	std::string const &bakDir;

	int intfd;

	std::future<void> watcherFut;
	std::vector<std::future<void>> futures;

	void remove(std::string const &path);
	void copy(std::string const &from, std::string const &to);

public:
	Backupper(std::string const &hot, std::string const &bak, Logger &logger);
	~Backupper();
};

#endif
