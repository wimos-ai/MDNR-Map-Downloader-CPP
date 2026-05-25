#pragma once
#define NOMINMAX

#include <functional>
#include <queue>
#include <atomic>
#include <thread>
#include <vector>
#include <semaphore>
#include <mutex>

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
	std::counting_semaphore<120000> sem{0};
	std::atomic_bool running;
	std::queue<Task> tasks;
	std::mutex tasks_mutex;
};

