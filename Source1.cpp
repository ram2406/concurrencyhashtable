#include <iostream>
#include <unordered_map>
#include <map>
#include <chrono>
#include <ctime>
#include <string>
#include <thread>
#include <mutex>
namespace gens {
const size_t IterationCount = 1000U;
const auto TimePoint = std::chrono::system_clock::now();
template <class Key>
Key GenerateKey () {
	static_assert(false, "not allowed");
}

template <class Value>
Value GenerateValue () {
	static_assert(false, "not allowed");
}

template <>
size_t GenerateKey<size_t> () {
	//static size_t unique_key = 0;
	//return ++unique_key;
	return 1 + rand() ;
}


template <>
std::string GenerateValue<std::string> () {
	auto end = std::chrono::system_clock::now();
	const auto& dif_sec_time =  std::chrono::duration_cast<std::chrono::nanoseconds>
                             (end-TimePoint).count();
	return std::to_string(dif_sec_time);
}



template <class Map, class Key, class Value, std::size_t N>
void InsertData(Map& map) {
	for(std::size_t iter=0; iter < N; ++iter) {
		map[GenerateKey<Key>()] = GenerateValue<Value>();
	}
}

template <class Map, class Key, class Value, std::size_t N>
void InsertDataMutex(Map& map) {
	static std::mutex mx;
	for(std::size_t iter=0; iter < N; ++iter) {
		std::unique_lock<std::mutex> lk(mx);
		map[GenerateKey<Key>()] = GenerateValue<Value>();
	}
}
}

namespace timer {
	void PrintTime () {
		static auto TimePoint = std::chrono::high_resolution_clock::now();
		const auto timePoint2 = std::chrono::high_resolution_clock::now();
		auto res = std::chrono::duration_cast<std::chrono::nanoseconds>
                             (timePoint2-TimePoint).count();
		if(!res) { return; }
		TimePoint = timePoint2;
		std::cout << "time: " << res * (0.000000001) << std::endl;
	}
}

int main () {
	timer::PrintTime();
	//std::this_thread::sleep_for(std::chrono::seconds(1)); 
	{
	std::unordered_map<size_t, std::string> map;
	gens::InsertData<decltype(map), size_t, std::string, gens::IterationCount>(map);
	timer::PrintTime();
	}

	{
	std::unordered_map<size_t, std::string> map;
	gens::InsertDataMutex<decltype(map), size_t, std::string, gens::IterationCount>(map);
	timer::PrintTime();
	}

	{
	std::map<size_t, std::string> map;
	gens::InsertData<decltype(map), size_t, std::string, gens::IterationCount>(map);
	timer::PrintTime();
	}
	system("pause");
	return 0;
}