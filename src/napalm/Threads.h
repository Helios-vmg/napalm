#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

#define LOCK_MUTEX(x) std::lock_guard<decltype(x)> lg_##__COUNT__(x)
#define UNIQUE_LOCK_MUTEX(x) std::unique_lock<decltype(x)> lg_##__COUNT__(x)

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
