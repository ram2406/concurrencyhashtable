#include "cht_defs.h"
#include "thread_pool.h"

namespace timer {
	std::chrono::system_clock::time_point& getLastTime() {
		static auto TimePoint = std::chrono::high_resolution_clock::now();
		return TimePoint;
	}
}

int main() { 

	
	system("pause");
	timer::PrintTime("finish");
	thread_pool().destroy_threads();		//for the love of deadlock, main-thread and other-threads
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return 0; 
}