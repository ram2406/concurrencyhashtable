/*
Concurrency Hash Table

* HashEntry - struct contains key/value and simple linked list
* HashTable - class implement an associative array, a structure that can map keys to values (HashEntry). Collisions is not trouble.
* ConcurrencyHashTable - based on HashTable with thread-safe

* HashMap<Key, Value, Sync> - easy interface for use
** Key - type of keys
** Value - type of values
** Sync - synchronize flag. If true - ConcurrencyHashTable be used, false - HashTable.
*/

#include <list>
#include <vector>
#include <memory>
#include <functional>
#include <map>


template <class Key, class Value>
struct HashEntry {

private:
	Key key;
	Value value;
public:
	HashEntry(Key key, Value value)
		: key(key), value(value)
	{
	}

	const Key& getKey() {
		return key;
	}

	Value& getValue() {
		return value;
	}



	typedef Key first_type;
	typedef Value second_type;
	typedef std::unique_ptr<HashEntry<Key, Value>> HashEntryPtr;

	HashEntryPtr next;		//linked list

	bool insert(const Key& key, const Value& value) {
		if(this->getKey() == key) {
			this->getValue() = value;
			return false;
		}
		if(!next) {
			next.reset(new HashEntry(key, value));
			return true;
		}
		return next->insert(key, value);
	}

	// top function for call recursive insert
	// return true if element inserted, or false if changed
	static bool insert(HashEntryPtr& entry, const Key& key, const Value& value) {
		if(entry->getKey() == key) {
			entry->getValue() = value;
			return false;
		}
		if(!entry) {
			entry.reset(new HashEntry(key, value));
			return true;
		}
		return entry->insert(key, value);
	}

	bool erase(const Key& key) {
		if(!next) {
			return false;
		}
		if(next->getKey() == key) {
			this->next = std::move(next->next);
			return true;
		}
		next->erase(key);
	}

	// top function for call recursive erase
	// return true if element finded by key and removed
	static bool erase(HashEntryPtr& entry, const Key& key) {
		if(entry->getKey() == key) {
			entry = std::move(entry->next);
			return true;
		}
		return entry->erase(key);
	}
	//calc elements count of linked list
	size_t size() const {
		if(!next) {
			return 0;
		}
		return 1 + next->size();
	}

	// top function for call recursive get
	static Value& get(HashEntryPtr& entry, const Key& key) {
		if(entry->getKey() == key) {
			return entry->getValue();
		}
		return entry->get(key);
	}

	Value& get(const Key& key) {
		if(this->getKey() == key) {
			return this->getValue();
		}
		if(!next) {
			next.reset(new HashEntry(key, Value()));
			return next->getValue();
		}
		return next->get(key);
	}

};

template<class Key, class Value, class SizeType = size_t>
class HashTable {
public:
	typedef HashEntry<Key, Value> hash_entry;
	typedef std::hash<Key> hash_type;
	typedef Key key_type;
	typedef hash_entry value_type;
private:
	std::vector<std::unique_ptr<HashEntry<Key, Value>>> table;
	const size_t length;
	SizeType current_size;
	const hash_type h;
	size_t calc_hash(const Key& key) const {
		return (h(key) % (length));
	}
	static const size_t DefaultCapacity = 1000;
public:
	HashTable()
		: table(DefaultCapacity), length(DefaultCapacity), current_size(0) {

	}
	HashTable(size_t capacity)
		: table(capacity), length(capacity), current_size(0) {

	}

	Value& get(const Key& key)  {
		const auto& hash = calc_hash(key);
		if (auto& entry = table[hash]) {
			const auto& before_size = entry->size();
			auto& value = hash_entry::get(entry, key);
			current_size += entry->size() - before_size;
			return value;
		}
		else {
			entry.reset(new hash_entry(key, Value()));
			++current_size;
			return entry->getValue();
		}
	}

	bool insert(const Key& key, const Value& value) {
		const auto& hash = calc_hash(key);
		auto& entry = table[hash];
		if(entry) {
			const auto& inserted = hash_entry::insert(entry, key, value);
			if(inserted) {
				++current_size;
			}
			return inserted;
		}
		else {
			entry.reset(new hash_entry(key, value));
			++current_size;
			return true;
		}

	}     


	Value& operator[] (const Key& key) { return get(key); }
	//const Value& operator[] (const Key& key) const { return get(key); }

	bool erase(const Key& key) {
		const auto& hash = calc_hash(key);
		if (auto& entry = table[hash]) {
			const auto& removed = hash_entry::erase(entry, key);
			if( removed ) {
				--current_size;
			}
			return removed;
		}
		return false;
	}
	size_t size() const { return current_size; }
};

#include <atomic>
#include <mutex>

//TODO: shared_mutex, spinlock_mutex
template <class Key, class Value, class Mutex = std::mutex>
class ConcurrencyHashTable
	: private HashTable<Key, Value, std::atomic_size_t> {
	typedef HashTable<Key, Value, std::atomic_size_t> base;
private:
	std::mutex mx;
public:
	typedef HashEntry<Key, Value> hash_entry;
	typedef Key key_type;
	typedef hash_entry value_type;

		ConcurrencyHashTable() {
		}
		ConcurrencyHashTable(size_t capacity)
			: base(capacity) {

		}

		Value& get(const Key& key)  {
			std::lock_guard<Mutex> lk(mx);
			return base::get(key);
		}

		bool insert(const Key& key, const Value& value) {
			std::lock_guard<Mutex> lk(mx);
			return base::insert(key, value);
		}     

		bool erase(const Key& key) {
			std::lock_guard<Mutex> lk(mx);
			return base::erase(key);
		}

		Value& operator[] (const Key& key) { return ConcurrencyHashTable::get(key); }
		//const Value& operator[] (const Key& key) const { return get(key); }


		size_t size() const { return base::size(); }
};

template <class Key, class Value, bool Sync = false>
class HashMap {
};

template <class Key, class Value>
class HashMap<Key,Value, true> 
	: public ConcurrencyHashTable<Key, Value>  {
	typedef ConcurrencyHashTable<Key, Value> base;
public:
	typedef HashEntry<Key, Value> hash_entry;
	typedef Key key_type;
	typedef hash_entry value_type;

		HashMap() {
		}
		HashMap(size_t capacity)
			: base(capacity) {

		}
};



template <class Key, class Value>
class HashMap<Key,Value, false> 
	: public HashTable<Key, Value>  {
	typedef HashTable<Key, Value> base;
public:
	typedef HashEntry<Key, Value> hash_entry;
	typedef Key key_type;
	typedef hash_entry value_type;

		HashMap() {
		}
		HashMap(size_t capacity)
			: base(capacity) {

		}
};
