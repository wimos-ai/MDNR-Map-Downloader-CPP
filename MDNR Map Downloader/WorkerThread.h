#ifndef WORKERTHREAD_H989327987
#define WORKERTHREAD_H989327987

#include <mutex>
#include <queue>
#include <functional>

#include "semaphore.h"

using Task = std::function<void(void)>;

/// <summary>
/// Queues tasks and completes them in the order that they were recieved.
/// NOTE: Exceptions fail silently in submitted tasks
/// </summary>
class WorkerThread
{
public:
	/// <summary>
	/// Terminates the thread. Current task is finished. Tasks remaining in the queue are discarded
	/// </summary>
	void kill();

	/// <summary>
	/// Adds a task to be done
	/// </summary>
	/// <param name="t">A task</param>
	void addTask(Task t);

	/// <summary>
	/// Disposes of all tasks in the queue without completing them.
	/// </summary>
	void clear();

	/// <summary>
	/// Constructor
	/// </summary>
	WorkerThread();

	/// <summary>
	/// When the destructor is called, the task in progress is finished, all remaining tasks are discarded, and the running thread is closed
	/// </summary>
	~WorkerThread();

private:

	//A mutex to lock the internal queue
	std::mutex lock;

	//A semaphore that matches the # of items in the queue
	semaphore sem;

	//A queue of tasks
	std::queue<Task> tasks;

	//A thread of opperation
	std::thread thd;

	//A condition variable to stop the thread
	volatile bool isRunning;

	//The driver code for each WorkerThread
	void threadMain();
};
#endif // !WORKERTHREAD_H989327987
