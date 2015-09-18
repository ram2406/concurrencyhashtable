#include "hash_table.h"
#include "cht_defs.h"
#include "thread_pool.h"

#include <assert.h>
#include <condition_variable>
#include <sstream>

static int behaviour_concurrency_test();
static int test_hash_table();
auto hack_hash_table = test_hash_table() + behaviour_concurrency_test();	//for run test

typedef HashMap<size_t, std::string> hash_table;

typedef HashMap<size_t, std::string, true> hash_table_thread_safe;

static 
int test_hash_table() {
	//simple behaviour testing 
	hash_table map(10);
	const std::string value = "test";
	const size_t key = 1000;
	map.insert(key+1, value);
	map.insert(key+2, value);
	map.insert(key, value);
	map.insert(key, value);	
	assert(map.size() == 3 && "size test failed");

	auto test = map.get(key);
	assert(!test.compare(value) && "compare value test failed");

	map[1] = std::string("dsdsa");
	map.erase(1);
	auto test2 = map[1];
	assert(test2.empty() && "default value test failed");

	//benchmark
	timer::initTimer();

	tests::test_cpp11_wo_mutex<	hash_table, gens::IterationCount >("my hash_table w/o mutex");
	
	tests::test_cpp11_with_mutex< hash_table, gens::IterationCount >("my hash_table with mutex");
	
	tests::test_cpp11_wo_mutex<	hash_table_thread_safe, gens::IterationCount >("my thread_safe_hash_table w/o mutex");

	return 0;
}



typedef HashMap<size_t, size_t, true> map_type;
static
int behaviour_concurrency_test() {
	const size_t IterationCount = 100;
	map_type map;
	std::condition_variable cond;
	std::mutex mx;

	auto f = [&](const map_type::key_type& key) -> std::string {
		std::unique_lock<std::mutex> lk(mx);
		lk.unlock();

		map.insert(std::this_thread::get_id().hash() + 1000, 1000);

		map_type::value_type::second_type res = map[key];
		map.erase(key);
		map.insert(std::this_thread::get_id().hash(), 1000);
		std::stringstream ss;
		ss << " searching value "
			<< res << " by key "
			<< key << std::endl;
		return ss.str();
	};

	map.insert(1, 1000);
	map.insert(5, 500);
	map.insert(10, 100);
	map.insert(50, 50);
	map.insert(100, 10);
	map.insert(500, 5);
	map.insert(1000, 1);
	typedef decltype(f) func_type;
	timer::initTimer();
	struct Functor {
		func_type fun;
		map_type::key_type key;
		Functor(func_type f, map_type::key_type k)
			: fun(f), key(k) {
		}
		void operator() () {
			fun(key);
		}
	};

	for (size_t ii = 0; ii < IterationCount; ++ii) {
		thread_pool().add_task(Functor(f, ii));
	}
	cond.notify_all();
	std::this_thread::sleep_for(std::chrono::microseconds(10));
	timer::PrintTime("my concurrency hash map test");
	

	return 0;
}
