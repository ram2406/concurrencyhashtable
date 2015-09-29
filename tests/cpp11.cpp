#include <iostream>
#include <unordered_map>
#include <map>
#include "cht_defs.h"

static int test_cpp11 ();
auto hack_cpp11 = test_cpp11();

typedef std::unordered_map<size_t, std::string> hash_table;
typedef std::map<size_t, std::string> map;

static
int test_cpp11 () {
	timer::initTimer();

	tests::test_cpp11_wo_mutex<	hash_table, gens::IterationCount >("unordered_map w/o mutex");
	
	tests::test_cpp11_with_mutex< hash_table, gens::IterationCount >("unordered_map with mutex");
	
	tests::test_cpp11_wo_mutex< map, gens::IterationCount >("simple std::map");
	
	
	return 0;
}

