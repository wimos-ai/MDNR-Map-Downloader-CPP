#pragma once


#include <functional>
#include <queue>
#include <atomic>
#include <vector>

#include "semaphore.h"

using Task = std::function<void(void)>;

class ThreadPool
{
public:
	ThreadPool(size_t num_thds = std::thread::hardware_concurrency());
	~ThreadPool();
	void submit_task(Task t);
private:
	std::vector<std::thread> worker_thds;
	size_t num_thds;
	semaphore sem;
	std::atomic_bool running;
	std::queue<Task> tasks;
	std::mutex tasks_mutex;
};

