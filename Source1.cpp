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
		{
			gens::InsertData<decltype(map), size_t, std::string, gens::IterationCount>(map);
			timer::PrintTime("unordered_map w/o mutex insert");
			const auto& s1 = map.size();
			std::cout << "  inserted count:" << s1 << std::endl;
		}
		{
			gens::RemoveData<decltype(map), size_t, std::string, gens::IterationCount>(map);
			timer::PrintTime("unordered_map w/o mutex remove");
			const auto& s2 = map.size();
			std::cout << "  removed count:" << s2 << std::endl;
		}
	}

	{
		std::unordered_map<size_t, std::string> map;
		{
			gens::InsertDataMutex<decltype(map), size_t, std::string, gens::IterationCount>(map);
			timer::PrintTime("unordered_map with mutex");
			const auto& s = map.size();
			std::cout << "  inserted count:" << s << std::endl;
		}
		{
			gens::RemoveData<decltype(map), size_t, std::string, gens::IterationCount>(map);
			timer::PrintTime("unordered_map with mutex remove");
			const auto& s = map.size();
			std::cout << "  removed count:" << s << std::endl;
		}
	}

	{
		std::map<size_t, std::string> map;
		{
			gens::InsertData<decltype(map), size_t, std::string, gens::IterationCount>(map);
			timer::PrintTime("simple std::map");
			const auto& s = map.size();
			std::cout << "  inserted count:" << s << std::endl;
		}
		{
			gens::RemoveData<decltype(map), size_t, std::string, gens::IterationCount>(map);
			timer::PrintTime("simple std::map remove");
			const auto& s = map.size();
			std::cout << "  removed count:" << s << std::endl;
		}
	}
	
	return 0;
}

auto hack2 = main2();