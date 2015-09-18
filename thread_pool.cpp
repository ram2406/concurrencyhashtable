#include "thread_pool.h"

ThreadPool& ThreadPool::instance() {
	static ThreadPool th_pool;
	return th_pool;
}


void ThreadPool::thread_loop() {
	auto& thp = thread_pool();
	while (thp.working_flag) {
		thread_pool_task task;
		thp.tasks.wait_and_pop(task);
		task();
	}
}

static void test_compile() {
	thread_pool().add_task([](){ int i = 1 + 1; });
}