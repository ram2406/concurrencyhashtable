#include "cht_defs.h"

namespace timer {
	std::chrono::system_clock::time_point& getLastTime() {
		static auto TimePoint = std::chrono::high_resolution_clock::now();
		return TimePoint;
	}
}

int main() { 

	
	system("pause");
	timer::PrintTime("finish");
	return 0; 
}