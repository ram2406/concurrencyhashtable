#include <iostream>
#include <unordered_map>
#include <map>
#include "Header.h"

static
int main2 () {
	timer::initTimer();

	//std::this_thread::sleep_for(std::chrono::seconds(1)); 
	{
	std::unordered_map<size_t, std::string> map;
	gens::InsertData<decltype(map), size_t, std::string, gens::IterationCount>(map);
	timer::PrintTime("unordered_map w/o mutex insert");
	gens::RemoveData<decltype(map), size_t, std::string, gens::IterationCount>(map);
	timer::PrintTime("unordered_map w/o mutex remove");
	}

	{
	std::unordered_map<size_t, std::string> map;
	gens::InsertDataMutex<decltype(map), size_t, std::string, gens::IterationCount>(map);
	timer::PrintTime("unordered_map with mutex");
	gens::RemoveData<decltype(map), size_t, std::string, gens::IterationCount>(map);
	timer::PrintTime("unordered_map with mutex remove");
	}

	{
	std::map<size_t, std::string> map;
	gens::InsertData<decltype(map), size_t, std::string, gens::IterationCount>(map);
	timer::PrintTime("simple std::map");
	gens::RemoveData<decltype(map), size_t, std::string, gens::IterationCount>(map);
	timer::PrintTime("simple std::map remove");
	}
	
	return 0;
}

auto hack2 = main2();