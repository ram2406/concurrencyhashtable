#include <iostream>
#include <unordered_map>
#include <map>
#include "cht_defs.h"

static int test_cpp11 ();
auto hack_cpp11 = test_cpp11();

static
int test_cpp11 () {
	timer::initTimer();

	//std::this_thread::sleep_for(std::chrono::seconds(1)); 
	{
		std::unordered_map<size_t, std::string> map;
		tests::test_cpp11_wo_mutex<decltype(map), size_t, std::string, gens::IterationCount>(map, "unordered_map w/o mutex");
	}

	{
		std::unordered_map<size_t, std::string> map;
		tests::test_cpp11_wo_mutex<decltype(map), size_t, std::string, gens::IterationCount>(map, "unordered_map with mutex");
	}

	{
		std::map<size_t, std::string> map;
		tests::test_cpp11_wo_mutex<decltype(map), size_t, std::string, gens::IterationCount>(map, "simple std::map");
	}
	
	return 0;
}

