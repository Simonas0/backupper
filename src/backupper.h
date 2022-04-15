#ifndef BACKUPPER_H
#define BACKUPPER_H

#include <filesystem>
#include <future>
#include <map>
#include <vector>

#include "logger.h"

class Backupper
{
	Logger *logger;

	struct File
	{
		char *path;
		std::time_t lastMod;
	};

	char *hotDir;
	char *bakDir;

	int intfd;

	std::future<void> watcherFut;
	std::vector<std::future<void>> futures;

	void remove(std::string *path);
	void copy(std::string *from, std::string *to);

public:
	Backupper(char *hot, char *bak, Logger *logger);
	~Backupper();
};

#endif
