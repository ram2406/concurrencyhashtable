#pragma once
#include <thread>
#include <vector>
#include <future>
#include <atomic>
#include <functional>
#include <iostream>
#include "thread_safe_queue.h"

template<class RetValue>
class ThreadPool {
public:
	typedef std::packaged_task<RetValue()> thread_pool_task;
private:
	friend class WaitThreads;
	std::atomic<bool> working_flag;
	std::vector<std::thread> threads;
	threadsafe_queue<thread_pool_task> tasks;


	ThreadPool()
		: working_flag(true)
	{
		size_t tc = std::thread::hardware_concurrency();
		for (size_t ti = 0; ti < tc; ++ti) {
			threads.emplace_back(std::thread(thread_loop));
		}
	}

	ThreadPool& operator=(const ThreadPool&) {} // = delete
	
public:
	~ThreadPool() { 
		destroy_threads();
	}
	static ThreadPool& instance();
	static void thread_loop();
	void destroy_threads() {
		working_flag = false;
		tasks.reset_cond();
		for (auto& th : threads) {
			th.join();
		}
		std::cout << " threads joined" << std::endl;
		threads.clear();
	}

	template<class Functor>
	void add_task(Functor fun) {
		auto task = std::make_shared<thread_pool_task>(fun);
		add_task(task);
	}

	void add_task(std::shared_ptr<thread_pool_task>& task) {
		tasks.push(task);
	}
};

typedef ThreadPool<void> SimpleThreadPool;
inline SimpleThreadPool& thread_pool() { return SimpleThreadPool::instance(); }