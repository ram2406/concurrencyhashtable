#include <catch.hpp>
#include "shared_mutex.h"
#include "hash_table/hash_map.h"
#include <hash_table/striping_lock_visitor.h>

typedef ConcurrencyHashTable<size_t, std::string, sm::shared_mutex, StripingLockVisitor<size_t, std::string, 3>> hash_table_thread_safe;


TEST_CASE("test of concurrency map") {
	hash_table_thread_safe smap(100);
	smap.insert(102, "100");
	smap.insert(10, "10");
	smap.insert(1, "1");
	smap.insert(5, "5");
	smap.insert(4, "4");

	auto res = smap.get(5);
	CHECK(res == "5");

	auto res2 = smap[4];
	CHECK(res2 == "4");

	smap.insert(51, "51");


	
}

