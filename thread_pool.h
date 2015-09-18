#pragma once
#include <thread>
#include <vector>
#include <future>
#include <atomic>
#include <functional>
#include "thread_safe_queue.h"

//typedef int thread_pool_task;
//typedef std::function<void()> thread_pool_task;
typedef std::packaged_task<void()> thread_pool_task;
class ThreadPool {
	std::atomic<bool> working_flag;
	std::vector<std::thread> threads;
	threadsafe_queue<thread_pool_task> tasks;


	ThreadPool()
		: working_flag(false)
		, threads(std::thread::hardware_concurrency()) {
		for (auto& th : threads) {
			th = std::thread(thread_loop);
		}
	}

	ThreadPool& operator=(const ThreadPool&) = delete;
	
public:
	~ThreadPool() { 
		working_flag = false;
		for (auto& th : threads) {
			th.join();
		}
	}
	static ThreadPool& instance();
	static void thread_loop();

	template<class Functor>
	void add_task(Functor fun) {
		auto task = std::make_shared<thread_pool_task>(fun);
		tasks.push(task);
	}
};

inline ThreadPool& thread_pool() { return ThreadPool::instance(); }