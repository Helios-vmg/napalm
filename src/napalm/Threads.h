#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

#define CONCAT(x, y) x##y
#define CONCAT2(x, y) CONCAT(x, y)
#define LOCK_MUTEX(x, y) std::lock_guard<decltype(x)> CONCAT2(lg_, __COUNTER__)(x)
#define LOCK_MUTEX2(x, y) my_lock_guard<decltype(x)> CONCAT2(lg_, __COUNTER__)(x, y)
#define UNIQUE_LOCK_MUTEX(x) std::unique_lock<decltype(x)> CONCAT2(lg_, __COUNTER__)(x)

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

template <typename T>
class mutex_wrapper{
	T mutex;
	const char *owner;
public:
	mutex_wrapper() = default;
	void lock(const char *owner = nullptr){
		this->mutex.lock();
		this->owner = owner;
	}
	void unlock(){
		this->owner = nullptr;
		this->mutex.unlock();
	}
};

template <typename T>
class my_lock_guard{
	T &mutex;
public:
	my_lock_guard(T &mutex, const char *owner): mutex(mutex){
		this->mutex.lock(owner);
	}
	~my_lock_guard(){
		this->mutex.unlock();
	}
};
