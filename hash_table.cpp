#include "hash_table.h"
#include "cht_defs.h"
#include <assert.h>

static int test_hash_table();
auto hack_hash_table = test_hash_table();

typedef HashMap<size_t, std::string> hash_table;

typedef HashMap<size_t, std::string, true> hash_table_thread_safe;

static 
int test_hash_table() {
	hash_table map(10);
	const std::string value = "test";
	const size_t key = 1000;
	map.insert(key+1, value);

	map.insert(key+2, value);
	map.insert(key, value);
	map.insert(key, value);
	
	assert(map.size() == 3 && "size test failed");

	auto test = map.get(key);
	map[1] = std::string("dsdsa");
	map.erase(1);
	auto test2 = map[1];
	assert(!test.compare(value) && "compare value test failed");

	timer::initTimer();

	tests::test_cpp11_wo_mutex<	hash_table, gens::IterationCount >("my hash_table w/o mutex");
	
	tests::test_cpp11_with_mutex< hash_table, gens::IterationCount >("my hash_table with mutex");
	
	tests::test_cpp11_wo_mutex<	hash_table_thread_safe, gens::IterationCount >("my thread_safe_hash_table w/o mutex");

	return 0;
}