#include <catch.hpp>
#include <cht_defs.h>
#include <hash_table/hash_map.h>
#include <hash_table/striping_lock_visitor.h>

typedef ConcurrencyHashTable<size_t, std::string, sm::shared_mutex, StripingLockVisitor<size_t, std::string, 10>> hash_table_thread_safe;



TEST_CASE("test of test 1") {
	hash_table_thread_safe smap;
	smap.insert(10, "10");
	smap.insert(1, "1");
	smap.insert(5, "5");
	smap.insert(4, "4");

	auto res = smap.get(5);
	CHECK(res == "5");
}