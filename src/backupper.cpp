#include <chrono>
#include <string>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "backupper.h"
#include "debug.h"

#define BAK_EXT ".bak"
#define BUF_SIZE 4096
#define DEL_PFX "delete_"
#define MAX_EVENTS 10

using namespace std::chrono_literals;

Backupper::Backupper(char *hot, char *bak, Logger *logger)
{
	DEBUG_MSG("Starting backupper");

	hotDir = hot;
	bakDir = bak;
	this->logger = logger;

	intfd = eventfd(0, 0);

	watcherFut = std::async(
		[&]()
		{
			epoll_event hotEvent, intEvent, events[MAX_EVENTS];

			hotEvent.events = intEvent.events = EPOLLIN | EPOLLET;
			hotEvent.data.fd = inotify_init1(IN_NONBLOCK);
			intEvent.data.fd = intfd;

			inotify_add_watch(hotEvent.data.fd, hotDir, IN_CLOSE_WRITE | IN_MOVED_TO);

			auto epfd = epoll_create1(0);
			epoll_ctl(epfd, EPOLL_CTL_ADD, hotEvent.data.fd, &hotEvent);
			epoll_ctl(epfd, EPOLL_CTL_ADD, intEvent.data.fd, &intEvent);

			while (true)
			{
				auto nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);

				bool hotAvailForRead = false, intAvailForRead = false;
				for (auto i = 0; i < nfds; ++i)
				{
					hotAvailForRead = hotAvailForRead || events[i].data.fd == hotEvent.data.fd;
					intAvailForRead = intAvailForRead || events[i].data.fd == intEvent.data.fd;
				}

				char buf[BUF_SIZE];
				ssize_t len;

				if (hotAvailForRead)
				{
					while (true)
					{
						len = read(hotEvent.data.fd, buf, sizeof(buf));

						if (len >= 0)
						{
							for (
								auto *event = (inotify_event *)buf;
								event < (inotify_event *)(buf + len);
								event = (inotify_event *)((char *)event + sizeof(inotify_event) + event->len))
							{
								if (strncmp(event->name, DEL_PFX, strlen(DEL_PFX)) == 0)
								{

									remove(&std::string(hotDir).append(event->name));
									remove(
										&std::string(bakDir)
											 .append(&event->name[strlen(DEL_PFX)])
											 .append(BAK_EXT));
								}
								else
								{
									copy(&std::string(hotDir).append(event->name),
										 &std::string(bakDir).append(event->name).append(BAK_EXT));
								}
							}
						}
						else
						{
							if (errno == EAGAIN || errno == EWOULDBLOCK)
							{
								break;
							}
						}
					}
				}

				if (intAvailForRead)
				{
					len = read(intEvent.data.fd, buf, sizeof(buf));
					if (len > 0)
					{
						break;
					}
				}

				// clean up futures periodically
				for (auto future = futures.begin(); future != futures.end();)
				{
					if (future->wait_for(0s) == std::future_status::ready)
					{
						future = futures.erase(future);
					}
					else
					{
						++future;
					}
				}
			}

			// make sure all asynchronous operations finished before exiting
			for (auto future = futures.begin(); future != futures.end(); ++future)
			{
				future->get();
			}
		});
}

Backupper::~Backupper()
{
	DEBUG_MSG("Stopping backupper");
	// any writing to intfd will stop backupper
	write(intfd, "whatever", 8);
	watcherFut.get();
}

void Backupper::remove(std::string *path)
{
	DEBUG_MSG("Deleting " << *path);

	futures.push_back(
		std::async([](Logger *logger, std::string path)
				   {
					   if (std::filesystem::remove(path))
					   {
						   (*logger).log(&path, delete_);
						   } },
				   logger, *path));
}

void Backupper::copy(std::string *from, std::string *to)
{
	DEBUG_MSG("Backing up " << *from << " to " << *to);

	futures.push_back(
		std::async([](Logger *logger, std::string from, std::string to)
				   {
					   bool existed = std::filesystem::exists(to);
					   std::filesystem::copy(from, to, std::filesystem::copy_options::overwrite_existing);
					   (*logger).log(&from, Action::backup);
					   (*logger).log(&to, existed ? Action::alter : Action::create); },
				   logger, *from, *to));
}
