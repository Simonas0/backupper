#include <iostream>

#include "backupper.h"
#include "cli.h"

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << argv[0] << ": Incorrect number of arguments." << std::endl;
		std::cerr << "Usage: " << argv[0] << " HOT_DIR BACKUP_DIR" << std::endl;
		std::cerr << "Backup files from HOT_DIR to BACKUP_DIR." << std::endl;

		return -1;
	}

	std::string hot(argv[1]), bak(argv[2]);

	if (!std::filesystem::is_directory(hot))
	{
		std::cerr << "Hot folder path does not refer to directory." << std::endl;
		return -1;
	}

	if (!std::filesystem::is_directory(bak))
	{
		std::cerr << "Backup folder path does not refer to directory." << std::endl;
		return -1;
	}

	// start logger
	Logger logger;

	// start backupper
	Backupper backupper(hot, bak, logger);

	// start cli
	Cli cli(logger);

	return 0;
}
