
#include <atomic>
#include <mutex>

template<class Key, class Value, class Mutex>
struct LockVisitor {
	typedef std::unique_lock<Mutex> lock;
	Mutex	mx;
	typedef HashEntry<Key, Value, LockVisitor> hash_entry;
	template<class EntryPtr>
	void entry_reset_value (EntryPtr& entry, const Key& key, const Value& value, size_t hash) {
		entry.reset(new hash_entry(key, value));
	}
	template<class EntryPtr>
	void entry_reset(EntryPtr& entry, const Key& key, size_t hash) {
		entry.reset(new hash_entry(key, Value()));
	}
	template<class EntryPtr>
	void entry_set(EntryPtr& entry, const Value& value, size_t hash) {
		entry->getValue() = value;
	}
	template<class EntryPtr>
	void entry_move(EntryPtr& entry, size_t hash) {
		entry = std::move(entry->getNext());
	}
	lock read_lock(size_t hash) {
		return lock(mx);
	}
};
