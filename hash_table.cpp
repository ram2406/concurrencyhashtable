#include "hash_table.h"
#include "cht_defs.h"
#include <assert.h>

static int test_hash_table();
auto hack_hash_table = test_hash_table();

typedef HashMap<size_t, std::string> hash_table;

static 
int test_hash_table() {
	HashMap<size_t, std::string> map(1000);
	const std::string value = "test";
	const size_t key = 1;
	map.insert(key, value);
	auto test = map.get(key);
	map[1] = std::string("dsdsa");
	map.erase(1);
	auto test2 = map[1];
	assert(!test.compare(value) && "test failed");

	timer::initTimer();

	tests::test_cpp11_wo_mutex<	hash_table, gens::IterationCount >("my hash_table w/o mutex");
	
	tests::test_cpp11_with_mutex< hash_table, gens::IterationCount >("my hash_table with mutex");
	

	return 0;
}