#include "shared_mutex.h"
#include "hash_table/hash_map.h"

typedef ConcurrencyHashTable<size_t, std::string, sm::shared_mutex> hash_table_thread_safe;

template<class Key, class Value>
struct LockVisitor<Key, Value, sm::shared_mutex> {
	typedef sm::shared_mutex Mutex;
	

	struct lock_read {
		Mutex &mx;
		lock_read(Mutex& mx) : mx(mx) {
			mx.lock_shared();
		}
		lock_read(lock_read&& l) : mx(l.mx) {
		}
		~lock_read() {
			mx.unlock_shared();
		}
	};

	typedef lock_read lock;

	Mutex mx;
	typedef HashEntry<Key, Value, LockVisitor> hash_entry;
	template<class EntryPtr>
	void entry_reset_value (EntryPtr& entry, const Key& key, const Value& value, size_t hash) {
		std::lock_guard<Mutex> lk(mx);
		entry.reset(new hash_entry(key, value));
	}
	template<class EntryPtr>
	void entry_reset(EntryPtr& entry, const Key& key, size_t hash) {
		std::lock_guard<Mutex> lk(mx);
		entry.reset(new hash_entry(key, Value()));
	}
	template<class EntryPtr>
	void entry_set(EntryPtr& entry, const Value& value, size_t hash) {
		std::lock_guard<Mutex> lk(mx);
		entry->getValue() = value;
	}
	template<class EntryPtr>
	void entry_move(EntryPtr& entry, size_t hash) {
		std::lock_guard<Mutex> lk(mx);
		entry = std::move(entry->getNext());
	}
	lock_read read_lock() {
		return lock_read(mx);
	}
};


int test_shared_mutex () {
	hash_table_thread_safe map;
	map[0] = "dsadad";
	map[1] = "dsadad1";
	map[2] = "dsadad2";
	map[3] = "dsadad3";
	auto s = map[2];
	return 0;
	
};

//static auto hack = test_shared_mutex();