#include <iostream>

#include "backupper.h"

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << argv[0] << ": Incorrect number of arguments." << std::endl;
		std::cerr << "Usage: " << argv[0] << " HOT_DIR BACKUP_DIR" << std::endl;
		std::cerr << "Backup files from HOT_DIR to BACKUP_DIR." << std::endl;

		return -1;
	}

	std::filesystem::path hotDir(argv[1]);
	std::filesystem::path bakDir(argv[2]);

	if (!std::filesystem::is_directory(hotDir))
	{
		std::cerr << "Hot folder path does not refer to directory." << std::endl;
		return -1;
	}

	if (!std::filesystem::is_directory(bakDir))
	{
		std::cerr << "Backup folder path does not refer to directory." << std::endl;
		return -1;
	}

	// start backupper
	Backupper backupper(hotDir, bakDir);

	// wait for input
	std::getchar();

	return 0;
}
