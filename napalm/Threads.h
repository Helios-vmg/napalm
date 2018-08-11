#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

class Event{
	bool signalled = false;
	std::mutex mutex;
	std::condition_variable cv;
	bool wait_impl();
public:
	void signal();
	void reset_and_wait();
	void wait();
	void reset();
	bool state();
};
