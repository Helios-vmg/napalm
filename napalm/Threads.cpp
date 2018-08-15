#include "Threads.h"

void Event::signal(){
	LOCK_MUTEX(this->mutex);
	this->signalled = true;
	this->cv.notify_one();
}

bool Event::state(){
	LOCK_MUTEX(this->mutex);
	return this->signalled;
}

void Event::reset(){
	LOCK_MUTEX(this->mutex);
	this->signalled = false;
}

bool Event::wait_impl(){
	std::unique_lock<std::mutex> lock(this->mutex);
	if (this->signalled){
		this->signalled = false;
		return false;
	}
	this->cv.wait(lock);
	return true;
}

void Event::wait(){
	while (this->wait_impl());
}

void Event::reset_and_wait(){
	this->wait_impl();
	this->wait();
}
