#include "WorkerThread.h"

void WorkerThread::addTask(Task task) {
	std::unique_lock<std::mutex> lck(this->lock);
	tasks.push(task);
	sem.release();
}

void WorkerThread::kill() {
	this->isRunning = false;
	sem.release();
}

void WorkerThread::clear() {
	std::unique_lock<std::mutex> lck(this->lock);
	while (!tasks.empty())
	{
		tasks.pop();
	}
}

void WorkerThread::threadMain()
{
	//While this thread is running
	while (this->isRunning) {

		//The value in sem matches the number of tasks in the queue. Makes waiting for a task very efficent
		sem.acquire();

		//If we are waiting for a task and the destructor is called, the semaphore must be released to allow the thread to exit
		//This checks for that condition
		if (!this->isRunning)
		{
			return;
		}

		//Take the top task off the queue
		Task tsk;
		{
			std::unique_lock<std::mutex> lck(this->lock);
			tsk = tasks.front();
			tasks.pop();

		}

		//Execute the task
		try {
			tsk();
		}
		catch (...) {
			//Probably should add logging here
		}


	}
}

WorkerThread::WorkerThread() :isRunning(true), tasks(), lock(), thd([this]() {this->threadMain(); }) {
}

WorkerThread::~WorkerThread() {
	kill();

	if (thd.joinable())
	{
		thd.join();
	}
}
