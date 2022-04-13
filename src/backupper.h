#ifndef BACKUPPER_H
#define BACKUPPER_H

#include <filesystem>
#include <future>
#include <map>
#include <vector>

class Backupper
{
	enum Action
	{
		backup,
		deleteHot,
		deleteBck
	};

	struct Operation
	{
		char *path;
		Action action;
	};

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

public:
	Backupper(char *hotDir, char *bakDir);
	~Backupper();
};

#endif
