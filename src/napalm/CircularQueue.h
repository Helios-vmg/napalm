#pragma once
#include "Threads.h"
#include <vector>

template <typename T>
class CircularQueue{
	std::vector<T> data;
	size_t head = 0, m_size = 0;
public:
	CircularQueue(size_t capacity){
		this->data.resize(capacity);
	}
	CircularQueue(const CircularQueue &) = default;
	CircularQueue &operator=(const CircularQueue &other) = default;
	CircularQueue(CircularQueue &&other){
		*this = std::move(other);
	}
	const CircularQueue &operator=(CircularQueue &&other){
		this->data = std::move(other.data);
		this->head = other.head;
		this->m_size = other.m_size;
		other.head = other.m_size = 0;
		return *this;
	}
	size_t size() const{
		return this->m_size;
	}
	size_t capacity() const{
		return this->data.size();
	}
	void push(T &&x){
		if (this->size() == this->capacity()){
			CircularQueue temp(this->capacity() * 2);
			while (this->size())
				temp.push(this->pop());
			*this = std::move(temp);
		}
		this->data[this->head + this->m_size++] = std::move(x);
	}
	T pop(){
		auto ret = std::move(this->data[this->head++]);
		this->m_size--;
		this->head %= this->capacity();
		return ret;
	}
};


template <typename T>
class ThreadSafeCircularQueue{
	std::mutex mutex;
	Event event;
	CircularQueue<T> queue;
public:
	ThreadSafeCircularQueue(size_t capacity): queue(capacity){}
	void push(T &&x){
		{
			LOCK_MUTEX(this->mutex, "ThreadSafeCircularQueue::push()");
			this->queue.push(std::move(x));
		}
		this->event.signal();
	}
	T pop(){
		while (true){
			{
				LOCK_MUTEX(this->mutex, "ThreadSafeCircularQueue::pop()");
				if (this->queue.size())
					return this->queue.pop();
			}
			this->event.wait();
		}
	}
};
