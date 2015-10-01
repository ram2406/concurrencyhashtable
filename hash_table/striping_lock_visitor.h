#include <array>
#include <shared_mutex.h>

template<class Key, class Value, size_t Level = 1>
struct StripingLockVisitor {
	size_t elementsPerMutex;
	StripingLockVisitor(size_t tableLength) : elementsPerMutex(tableLength / Level) {

	}

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

	std::array<Mutex, Level> mxs;

	Mutex& get_mx(size_t hash) {
		const auto& mxi = hash / elementsPerMutex;
		return mxs[mxi < Level ? mxi : Level - 1];
	}

	typedef HashEntry<Key, Value, StripingLockVisitor> hash_entry;
	template<class EntryPtr>
	void entry_reset_value (EntryPtr& entry, const Key& key, const Value& value, size_t hash) {
		std::lock_guard<Mutex> lk(get_mx(hash));
		entry.reset(new hash_entry(key, value));
	}
	template<class EntryPtr>
	void entry_reset(EntryPtr& entry, const Key& key, size_t hash) {
		std::lock_guard<Mutex> lk(get_mx(hash));
		entry.reset(new hash_entry(key, Value()));
	}
	template<class EntryPtr>
	void entry_set(EntryPtr& entry, const Value& value, size_t hash) {
		std::lock_guard<Mutex> lk(get_mx(hash));
		entry->getValue() = value;
	}
	template<class EntryPtr>
	void entry_move(EntryPtr& entry, size_t hash) {
		std::lock_guard<Mutex> lk(get_mx(hash));
		entry = std::move(entry->getNext());
	}
	lock_read read_lock(size_t hash) {
		return lock_read(get_mx(hash));
	}
};
