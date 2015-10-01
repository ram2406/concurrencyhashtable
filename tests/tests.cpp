#include <catch.hpp>
#include <shared_mutex.h>
#include <hash_table/hash_map.h>
#include <hash_table/striping_lock_visitor.h>

#include <map>
#include <cht_defs.h>
#include <thread_pool.h>

typedef ConcurrencyHashTable<size_t, std::string, sm::shared_mutex, StripingLockVisitor<size_t, std::string, 10>> hash_table_thread_safe;

TEST_CASE("test of concurrency map") {
	hash_table_thread_safe smap(100);
	INFO("checking insert operations work");
	smap.insert(102, "100");
	smap.insert(10, "10");
	smap.insert(1, "1");
	smap.insert(5, "5");
	smap.insert(4, "4");

	auto res = smap.get(5);
	CHECK(res == "5");

	INFO("checking index-operator work")
	auto res2 = smap[4];
	CHECK(res2 == "4");

	smap.insert(51, "51");

	INFO("checking erase elements");
	smap.erase(102);
	smap.erase(51);

	INFO("checking iterations work");
	size_t sum = 0;
	for (auto& iter : smap) {
		sum += iter.getKey();
	}
	CHECK(sum == (10 + 1 + 5 + 4));
}

template < class Map, size_t IterCount>
double test_map(const std::string& text) {
	typedef typename Map::key_type Key;
	typedef typename Map::value_type::second_type Value;
	Map map;
	double full_time = 0;
	size_t inserted_count = 0;
	{
		gens::InsertData<Map, Key, Value, IterCount>(map);
		full_time += timer::PrintTime(text + " insert");
		inserted_count = map.size();
		std::cout << "  inserted count: " << inserted_count << std::endl;
	}
	{
		gens::RemoveData<Map, Key, Value, IterCount>(map);
		full_time += timer::PrintTime(text + " remove");
		const auto& s2 = inserted_count - map.size();
		std::cout << "  removed count: " << s2 << std::endl;
	}
	return full_time;
}

template < class Map, size_t IterCount>
double test_hash_table(const std::string& text) {
	typedef typename Map::key_type Key;
	typedef typename Map::value_type::second_type Value;
	double full_time = 0;
	size_t inserted_count = 0;
	Map map(IterCount);
	{
		gens::InsertData2<Map, Key, Value, IterCount>(map);
		full_time += timer::PrintTime(text + " insert");
		inserted_count = map.size();
		std::cout << "  inserted count: " << inserted_count << std::endl;
	}
	{
		gens::RemoveData2<Map, Key, Value, IterCount>(map);
		full_time += timer::PrintTime(text + " remove");
		const auto& s2 = inserted_count - map.size();
		std::cout << "  removed count: " << s2 << std::endl;
	}
	return full_time;
}

TEST_CASE("speed test of concurrency map") {
	typedef hash_table_thread_safe hash_table;
	typedef std::map<size_t, std::string> map;

	timer::initTimer();
	std::cout << " == benchmark ==> " << std::endl;
	auto time1 = test_hash_table< hash_table, gens::IterationCount >("speed of ConcurrencyHashTable with StripingLockVisitor");
	std::cout << "  full time: " << time1 << "s" << std::endl;
	auto time2 = test_map< map, gens::IterationCount >("speed of std::map");
	std::cout << "  full time: " << time2 << "s" << std::endl;


	INFO("checking full time insert/remove operations");
	CHECK(time1 < time2);

	std::cout << " my concurrency hash table faster on " << time2 / time1 << std::endl;

	std::cout << " <== benchmark == " << std::endl;

}



TEST_CASE("test of concurrency map parallel") {
	
	typedef hash_table_thread_safe Map;
	typedef Map::key_type Key;
	typedef Map::value_type::second_type Value;
	
	auto sh_map = std::make_shared<Map>( gens::IterationCount); //not thread_safe, but for test allowed
	thread_pool().add_task([=]()
	{
		std::cout << " start insert " << std::endl;
		gens::InsertData2<Map, Key, Value, gens::IterationCount>(*sh_map);
	});
	thread_pool().add_task([=]()
	{
		std::cout << " start erase " << std::endl;
		gens::RemoveData2<Map, Key, Value, gens::IterationCount>(*sh_map);
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	INFO("checking deadlock and any one result");
	CHECK(sh_map->size() > 0);
}