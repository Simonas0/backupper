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
		Action action;
		std::filesystem::path path;
	};

	struct File
	{
		std::filesystem::path path;
		std::time_t lastMod;
	};

	std::filesystem::path hotDir;
	std::filesystem::path bakDir;

	std::atomic<bool> stopFlag = false;
	std::future<void> mainFut;

	std::map<std::filesystem::file_time_type, Operation> pending;

	std::vector<std::filesystem::path> inUse;
	std::mutex inUse_mutex;

	// last known modify time of each hot file
	std::map<std::string, std::filesystem::file_time_type> hotFiles;

	void removeFile(std::filesystem::path path);
	void backupFile(std::filesystem::path path);

public:
	Backupper(std::filesystem::path hotDir, std::filesystem::path bakDir);
	~Backupper();
};

#endif
