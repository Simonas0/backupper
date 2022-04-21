#include <string>
#include <thread>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "backupper.h"
#include "debug.h"
#include "time.h"

#define BAK_EXT ".bak"
#define BUF_SIZE 4096
#define DEL_PFX "delete_"
#define MAX_EVENTS 10
#define PENDING_FILE "pending"

using namespace std::chrono_literals;

Backupper::Backupper(std::string const &hot, std::string const &bak, Logger &logger)
	: hotDir(hot), bakDir(bak), logger(logger)
{
	DEBUG_MSG("Starting backupper");

	intfd = eventfd(0, 0);

	if (intfd == -1)
	{
		perror("eventfd");
		exit(EXIT_FAILURE);
	}

	std::ifstream pending(PENDING_FILE);
	std::string line;

	while (std::getline(pending, line))
	{
		const auto idx = line.find(' ');
		remove(line.substr(idx + 1), Time::stringToTime(line.substr(0, idx)));
	}

	watcherFut = std::async(
		[&]()
		{
			epoll_event hotEvent, intEvent, events[MAX_EVENTS];

			hotEvent.events = intEvent.events = EPOLLIN | EPOLLET;
			hotEvent.data.fd = inotify_init1(IN_NONBLOCK);

			if (hotEvent.data.fd == -1)
			{
				perror("inotify_init1");
				exit(EXIT_FAILURE);
			}

			intEvent.data.fd = intfd;

			if (inotify_add_watch(hotEvent.data.fd, hotDir.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO) == -1)
			{
				perror("inotify_add_watch");
				exit(EXIT_FAILURE);
			}

			const auto epfd = epoll_create1(0);

			if (epfd == -1)
			{
				perror("epoll_create1");
				exit(EXIT_FAILURE);
			}

			if (epoll_ctl(epfd, EPOLL_CTL_ADD, hotEvent.data.fd, &hotEvent) == -1 ||
				epoll_ctl(epfd, EPOLL_CTL_ADD, intEvent.data.fd, &intEvent) == -1)
			{
				perror("epoll_ctl");
				exit(EXIT_FAILURE);
			}

			while (true)
			{
				const auto nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);

				if (nfds == -1)
				{
					if (errno == EINTR)
					{
						continue;
					}
					perror("epoll_wait");
					exit(EXIT_FAILURE);
				}

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
								const auto *event = (inotify_event *)buf;
								event < (inotify_event *)(buf + len);
								event = (inotify_event *)((char *)event + sizeof(inotify_event) + event->len))
							{
								if (strncmp(event->name, DEL_PFX, strlen(DEL_PFX)) == 0)
								{
									std::string nameWoPfx = &event->name[strlen(DEL_PFX)];
									auto idx = nameWoPfx.find('_');

									DEBUG_MSG("Time to delete " << nameWoPfx.substr(0, idx));

									if (idx != std::string::npos)
									{
										auto time = Time::stringToTime(nameWoPfx.substr(0, idx), true);
										remove(std::string(hotDir).append(event->name), time);
										remove(std::string(bakDir)
												   .append(nameWoPfx.substr(idx + 1))
												   .append(BAK_EXT),
											   time);
									}
									else
									{
										remove(std::string(hotDir).append(event->name));
										remove(std::string(bakDir)
												   .append(&event->name[strlen(DEL_PFX)])
												   .append(BAK_EXT));
									}
								}
								else
								{
									copy(std::string(hotDir).append(event->name),
										 std::string(bakDir).append(event->name).append(BAK_EXT));
								}
							}
						}
						else
						{
							if (errno == EAGAIN || errno == EWOULDBLOCK)
							{
								break;
							}
							else
							{
								perror("read");
								exit(EXIT_FAILURE);
							}
						}
					}
				}

				if (intAvailForRead)
				{
					break;
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

				for (auto op = pendingOps.begin(); op != pendingOps.end();)
				{
					if (op->future.wait_for(0s) == std::future_status::ready)
					{
						op = pendingOps.erase(op);
					}
					else
					{
						++op;
					}
				}
			}

			// make sure all asynchronous operations finished before exiting
			for (auto future = futures.begin(); future != futures.end(); ++future)
			{
				future->get();
			}

			// save pending files
			std::ofstream pending(PENDING_FILE);
			deleteFuturesMutex.lock();
			for (auto op = pendingOps.begin(); op != pendingOps.end(); ++op)
			{
				if (op->future.wait_for(0s) != std::future_status::ready)
				{
					pending << Time::timeToString(op->time) << " " << op->path << std::endl;
				}
			}
		});
}

Backupper::~Backupper()
{
	DEBUG_MSG("Stopping backupper");
	// any writing to intfd will stop main loop

	if (write(intfd, "whatever", 8) == -1)
	{
		perror("write");
		exit(EXIT_FAILURE);
	}
	watcherFut.get();

	// stopping deferred deletion threads
	std::unique_lock<std::mutex> lock(stopFlagMutex);
	stopCV.notify_all();
}

void Backupper::remove(std::string const &path)
{
	DEBUG_MSG("Deleting " << path);

	futures.push_back(
		std::async([](Logger *logger, std::string path)
				   {
					   if (std::filesystem::remove(path))
					   {
						   (*logger).log(path, delete_);
						   } },
				   &logger, path));
}

void Backupper::remove(std::string const &path, std::chrono::system_clock::time_point const &time)
{
	pendingOps.push_back(PendingOp{
		.future = std::async([](Logger *logger, std::string path,
								std::chrono::system_clock::time_point const &time, std::mutex *pendingMutex,
								std::mutex *stopMutex, std::condition_variable *cv)
							 {
								std::unique_lock<std::mutex> lock(*stopMutex);
								if((*cv).wait_until(lock, time) == std::cv_status::no_timeout)
								{
									return;
								}

								 (*pendingMutex).lock();
								 if (std::filesystem::remove(path))
								 {
									 (*logger).log(path, delete_);
							 }
							 (*pendingMutex).unlock(); },
							 &logger, path, time, &deleteFuturesMutex, &stopFlagMutex, &stopCV),
		.path = path,
		.time = time});
}

void Backupper::copy(std::string const &from, std::string const &to)
{
	DEBUG_MSG("Backing up " << from << " to " << to);

	futures.push_back(
		std::async([](Logger *logger, std::string from, std::string to)
				   {
					   const bool existed = std::filesystem::exists(to);
					   std::filesystem::copy(from, to, std::filesystem::copy_options::overwrite_existing);
					   (*logger).log(from, Action::backup);
					   (*logger).log(to, existed ? Action::alter : Action::create); },
				   &logger, from, to));
}
