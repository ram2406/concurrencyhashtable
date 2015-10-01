#include "shared_mutex.h"
#include "hash_table/hash_map.h"
#include <hash_table\striping_lock_visitor.h>

typedef ConcurrencyHashTable<size_t, std::string, sm::shared_mutex, StripingLockVisitor<size_t, std::string, 10>> hash_table_thread_safe;


int test_shared_mutex () {
	hash_table_thread_safe map;
	map[0] = "dsadad";
	map[1] = "dsadad1";
	map[2] = "dsadad2";
	map[3] = "dsadad3";
	auto s = map[2];
	return 0;
	
};

//static auto hack = test_shared_mutex();