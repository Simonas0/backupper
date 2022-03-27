#include <iostream>
#include <string.h>
#include <thread>

#include "backupper.h"
#include "debug.h"

#define DEL_PFX "delete_"
#define BAK_EXT ".bak"
#define CHK_INT std::chrono::seconds(1)

Backupper::Backupper(std::filesystem::path hotDir, std::filesystem::path bakDir)
{
	DEBUG_MSG("Starting backupper");

	this->hotDir = hotDir;
	this->bakDir = bakDir;

	mainFut = std::async(
		[&]()
		{
			while (!stopFlag)
			{
				for (auto entry : std::filesystem::directory_iterator(hotDir))
				{
					std::string filename = entry.path().filename().string();

					if (hotFiles.find(filename)->second != entry.last_write_time() && std::filesystem::is_regular_file(entry))
					{
						hotFiles.erase(filename);
						hotFiles.insert(std::pair(filename, entry.last_write_time()));

						if (filename.rfind(DEL_PFX, 0) == 0)
						{
							DEBUG_MSG("Adding " << hotDir.string() << filename << " and " << bakDir.string() << filename.substr(strlen(DEL_PFX)) + BAK_EXT << " to pending for deletion");

							pending.insert(std::pair(entry.last_write_time(), Operation{.action = deleteHot, .path = filename}));
							pending.insert(std::pair(entry.last_write_time(), Operation{.action = deleteBck, .path = filename.substr(strlen(DEL_PFX)) + BAK_EXT}));
						}
						else
						{
							DEBUG_MSG("Adding " << hotDir.string() << filename << " to pending for backing up");

							pending.insert(std::pair(entry.last_write_time(), Operation{.action = Action::backup, .path = filename}));
						}
					}
				}
				std::this_thread::sleep_for(CHK_INT);
			}
		});
}

Backupper::~Backupper()
{
	DEBUG_MSG("Stopping backupper");
	stopFlag = true;
	mainFut.get();
}

void Backupper::removeFile(std::filesystem::path path)
{
	DEBUG_MSG("Deleting " << path);
}

void Backupper::backupFile(std::filesystem::path path)
{
	DEBUG_MSG("Copying " << path << " to " << bakDir / (path.filename().string() + BAK_EXT));
}
