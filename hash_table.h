#include <list>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <atomic>

template <class Key, class Value>
class HashEntry {

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

	HashEntryPtr next;

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

	static bool erase(HashEntryPtr& entry, const Key& key) {
		if(entry->getKey() == key) {
			entry = std::move(entry->next);
			return true;
		}
		return entry->erase(key);
	}
	size_t size() const {
		if(!next) {
			return 0;
		}
		return 1 + next->size();
	}
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

template<class Key, class Value>
class HashMap {
public:
	typedef HashEntry<Key, Value> hash_entry;
	typedef std::hash<Key> hash_type;
	typedef Key key_type;
	typedef hash_entry value_type;
private:
	std::vector<std::unique_ptr<HashEntry<Key, Value>>> table;
	size_t length;
	std::atomic_size_t current_size;
	hash_type h;
	size_t find(const Key& key) const {
		size_t hash = (h(key) % (length ));
		while (table[hash] != nullptr && table[hash]->getKey() != key)
			hash = ((hash + 1) % (length ));
		return hash;
	}
	static const size_t DefaultCapacity = 1000;
public:
	HashMap()
		: table(DefaultCapacity), length(DefaultCapacity), current_size(0) {

	}
	HashMap(size_t capacity)
		: table(capacity), length(capacity), current_size(0) {

	}

	Value& get(const Key& key)  {
		const auto& hash = find(key);
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
		const auto& hash = find(key);
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
			return true;
		}

	}     


	Value& operator[] (const Key& key) { return get(key); }
	const Value& operator[] (const Key& key) const { return get(key); }

	bool erase(const Key& key) {
		const auto& hash = find(key);
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
