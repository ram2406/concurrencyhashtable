#include "cht_defs.h"
#include "thread_pool.h"

#define CATCH_CONFIG_RUNNER  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

namespace timer {
	std::chrono::system_clock::time_point& getLastTime() {
		static auto TimePoint = std::chrono::high_resolution_clock::now();
		return TimePoint;
	}
}



int main(int argc,  char* const argv[]) { 

	int result = Catch::Session().run(argc, argv);
	system("pause");
	timer::PrintTime("finish");
	thread_pool().destroy_threads();		//for the love of deadlock, main-thread and other-threads
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return result; 
}

/*
#if defined (WIN32)
#include <Windows.h>
int CALLBACK WinMain(HINSTANCE,HINSTANCE,LPSTR,int) {
	return main();
}
#endif
*/