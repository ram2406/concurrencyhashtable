#include <iostream>
#include <map>
#include <sstream>

#include <future>


#include <cds/container/striped_map/std_map.h>
#include <cds/container/striped_map.h>

namespace cc = cds::container;
typedef cc::StripedMap < std::map<int, int>> map_type;

int main() {
	
	map_type map;
	std::condition_variable cond;
	std::mutex mx;

	auto f = [&](const map_type::key_type& key) -> std::string {
		std::unique_lock<std::mutex> lk(mx);
		lk.unlock();

		std::this_thread::sleep_for(std::chrono::microseconds(100));
		map.insert(std::this_thread::get_id().hash() + 1000, 1000);
		
		map_type::value_type::second_type res = 0;
		map.find(key, [&res](map_type::value_type value) { res = value.second; });
		map.erase(key);
		map.insert(std::this_thread::get_id().hash(), 1000);
		std::stringstream ss;
		ss << " searching value "
			<< res << " by key "
			<< key << std::endl;
		return ss.str();
	};

	map.insert(1, 1000);
	map.insert(10, 100);
	map.insert(100, 10);
	map.insert(1000, 1);
	
	std::vector<std::future<std::string>> vec_ft(4);
	std::size_t factor = 1;
	for (auto &ft : vec_ft) {
		ft = std::async(f, factor);
		factor *= 10;
	}
	cond.notify_all();
	for (auto &ft : vec_ft) {
		bool ft_ready = ft.wait_for(std::chrono::microseconds(1)) == std::future_status::ready;
		if (ft_ready) {
			std::cout << ft.get();
		}
	}

	system("pause");

	return 0;
}