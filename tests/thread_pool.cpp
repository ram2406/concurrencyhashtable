#include "thread_pool.h"

template<>

SimpleThreadPool& SimpleThreadPool::instance() {
	static SimpleThreadPool th_pool;
	return th_pool;
}


void SimpleThreadPool::thread_loop() {
	auto& thp = thread_pool();
	while (true) {
		auto task = thp.tasks.wait_and_pop();
		if (!thp.working_flag) {
			std::cout << " thread exit " << std::endl;
			return;
		}
		if (task) {
			(*task)();
		}
	}
	
}

static void test_compile() {
	thread_pool().add_task([](){ int i = 1 + 1; });
}

static int test_thread_pool();
auto hack_test_thread_pool = test_thread_pool();

static int test_thread_pool() {
	auto task = []() { std::this_thread::sleep_for(std::chrono::microseconds(1)); };
	thread_pool().add_task(task);
	thread_pool().add_task(task);
	thread_pool().add_task(task);
	thread_pool().add_task(task);

	return 0;
}