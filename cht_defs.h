#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <thread>
#include <mutex>

#if _MSC_VER > 1800
#define CDS_COMPILER_SUPPORT 1
#else
//#define CDS_COMPILER_SUPPORT 0
#endif

namespace gens {
	const size_t IterationCount = 1000U;
	const auto TimePoint = std::chrono::system_clock::now();
	template <class Key>
	Key GenerateKey() {
		static_assert(false, "not allowed");
	}

	template <class Value>
	Value GenerateValue() {
		static_assert(false, "not allowed");
	}

	template <> inline
	size_t GenerateKey<size_t>() {
		//static size_t unique_key = 0;
		//return ++unique_key;
		return 1 + rand() % IterationCount;
	}


	template <> inline
	std::string GenerateValue<std::string>() {
		auto end = std::chrono::system_clock::now();
		const auto& dif_sec_time = std::chrono::duration_cast<std::chrono::nanoseconds>
			(end - TimePoint).count();
		return std::to_string(dif_sec_time);
	}



	template <class Map, class Key, class Value, std::size_t N>
	void InsertData(Map& map) {
		for (std::size_t iter = 0; iter < N; ++iter) {
			map[GenerateKey<Key>()] = GenerateValue<Value>();
		}
	}

	template <class Map, class Key, class Value, std::size_t N>
	void RemoveData(Map& map) {
		for (std::size_t iter = 0; iter < N; ++iter) {
			map.erase(GenerateKey<Key>());
		}
	}

	template <class Map, class Key, class Value, std::size_t N>
	void InsertData2(Map& map) {
		for (std::size_t iter = 0; iter < N; ++iter) {
			map.insert(GenerateKey<Key>(), GenerateValue<Value>());
		}
	}

	template <class Map, class Key, class Value, std::size_t N>
	void RemoveData2(Map& map) {
		for (std::size_t iter = 0; iter < N; ++iter) {
			map.erase(GenerateKey<Key>());
		}
	}

	template <class Map, class Key, class Value, std::size_t N>
	void InsertDataMutex(Map& map) {
		static std::mutex mx;
		for (std::size_t iter = 0; iter < N; ++iter) {
			std::unique_lock<std::mutex> lk(mx);
			map[GenerateKey<Key>()] = GenerateValue<Value>();
		}
	}
}

namespace timer {

	std::chrono::system_clock::time_point& getLastTime();

	inline 
	void initTimer() { getLastTime(); }

	

	inline
	void PrintTime(const std::string& text) {
		auto& TimePoint = getLastTime();
		const auto timePoint2 = std::chrono::high_resolution_clock::now();
		auto res = std::chrono::duration_cast<std::chrono::nanoseconds>
			(timePoint2 - TimePoint).count();
		if (!res) { return; }
		TimePoint = timePoint2;
		std::cout << text  << ", time: " << res * (0.000000001) << std::endl;
	}
}