#ifndef BACKUPPER_H
#define BACKUPPER_H

#include <chrono>
#include <filesystem>
#include <future>
#include <map>
#include <vector>

#include "logger.h"

class Backupper
{
	struct PendingOp
	{
		std::future<void> future;
		std::string path;
		std::chrono::system_clock::time_point time;
	};

	Logger &logger;

	std::string const &hotDir;
	std::string const &bakDir;

	int intfd;

	std::future<void> watcherFut;
	std::vector<std::future<void>> futures;

	std::vector<PendingOp> pendingOps;
	std::mutex deleteFuturesMutex;
	std::mutex stopFlagMutex;
	std::condition_variable stopCV;


	void copy(std::string const &from, std::string const &to);
	void remove(std::string const &path);
	void remove(std::string const &path, std::chrono::system_clock::time_point const &time);

public:
	Backupper(std::string const &hot, std::string const &bak, Logger &logger);
	~Backupper();
};

#endif
