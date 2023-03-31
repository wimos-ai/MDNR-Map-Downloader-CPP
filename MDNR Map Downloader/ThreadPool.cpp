#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t num_thds): num_thds(num_thds)
{
	worker_thds.reserve(num_thds);

	for (size_t x = 0; x < num_thds; x++) {
		std::thread t([this]() {
			while (this->running) {
				this->sem.acquire();
				this->tasks_mutex.lock();
				Task t;
				if (!tasks.empty())
				{
					t = this->tasks.front();
					this->tasks.pop();
				}
				else {
					t = []() {};
				}

				this->tasks_mutex.unlock();
				t();
			}

			}
		);
		worker_thds.push_back(std::move(t));
	}


}

ThreadPool::~ThreadPool()
{
	//Signal to threads to end
	this->running = false;

	//Submit empty tasks for them to execute
	for (size_t x = 0; x < num_thds * 2; x++) {
		this->submit_task([]() {});
	}

	//Join the thread
	for (size_t x = 0; x < num_thds; x++) {
		if (worker_thds[x].joinable())
		{
			worker_thds[x].join();
		}
		
	}
}

void ThreadPool::submit_task(Task t)
{
	std::lock_guard<std::mutex> lck(tasks_mutex);
	tasks.push(t);
	sem.release();
}
