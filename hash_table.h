#pragma once
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



template <class Key, class Value, class Visitor>
struct HashEntry {
	typedef Key first_type;
	typedef Value second_type;
	typedef std::unique_ptr<HashEntry<Key, Value, Visitor>> HashEntryPtr;
private:
	Key key;
	Value value;
	HashEntryPtr next;		//linked list
public:
	HashEntry(Key key, Value value)
		: key(key), value(value)
	{
	}

	const Key& getKey() const {
		return key;
	}

	Value& getValue() {
		return value;
	}
	HashEntryPtr& getNext() {
		return next;
	}

	bool insert(const Key& key, const Value& value, Visitor& v) {
		if(this->getKey() == key) {
			v.entry_set(this, value);
			return false;
		}
		if(!next) {
			v.entry_reset(next, key, value);
			return true;
		}
		return next->insert(key, value, v);
	}

	// top function for call recursive insert
	// return true if element inserted, or false if changed
	static bool insert(HashEntryPtr& entry, const Key& key, const Value& value, Visitor& v) {
		if(entry->getKey() == key) {
			v.entry_set(entry, value);
			return false;
		}
		if(!entry) {
			v.entry_reset(entry, key, value);
			return true;
		}
		return entry->insert(key, value, v);
	}

	bool erase(const Key& key, Visitor& v) {
		if(!next) {
			return false;
		}
		if(next->getKey() == key) {
			v.entry_move(next);
			return true;
		}
		return next->erase(key, v);
	}

	// top function for call recursive erase
	// return true if element finded by key and removed
	static bool erase(HashEntryPtr& entry, const Key& key, Visitor& v) {
		if(entry->getKey() == key) {
			v.entry_move(entry);
			return true;
		}
		return entry->erase(key,v);
	}
	//calc elements count of linked list
	size_t size() const {
		if(!next) {
			return 0;
		}
		return 1 + next->size();
	}

	// top function for call recursive get
	static Value& get(HashEntryPtr& entry, const Key& key, Visitor& v) {
		if(entry->getKey() == key) {
			return entry->getValue();
		}
		return entry->get(key,v);
	}

	Value& get(const Key& key, Visitor& v) {
		if(this->getKey() == key) {
			return this->getValue();
		}
		if(!next) {
			v.entry_reset(next, key);
			return next->getValue();
		}
		return next->get(key, v);
	}

};


template<class HTable>
class HashTableIterator : public std::iterator<std::output_iterator_tag, typename HTable::hash_entry>
{
	typedef typename HTable::hash_entry HEntry;
	HTable& table;
	HEntry* entry;
public:
	HashTableIterator(HTable& table, HEntry* entry) : table(table), entry(entry) {}
  HashTableIterator(const HashTableIterator& it) : table(it.table), entry(it.entry) {}

  HashTableIterator& operator--();
  HashTableIterator operator--(int);

  HashTableIterator& operator++();
  HashTableIterator operator++(int);

  HEntry& operator*();

  bool operator==(const HashTableIterator& rhs) { return &table == &rhs.table && entry == rhs.entry; }
  bool operator!=(const HashTableIterator& rhs) { return !this->operator==(rhs); }

  
};

template<class Key, class Value, class Visitor, class SizeType = size_t>
class HashTable {
	
public:
	typedef HashEntry<Key, Value, Visitor> hash_entry;
	typedef std::unique_ptr<hash_entry> hash_entry_ptr;
	typedef std::hash<Key> hash_type;
	typedef Key key_type;
	typedef hash_entry value_type;
private:
	friend class HashTableIterator<HashTable>;
	std::vector<std::unique_ptr<hash_entry>> table;
	const size_t length;
	SizeType current_size;
	const hash_type h;
protected:
	Visitor visitor;
	static const size_t DefaultCapacity = 1000;

	size_t calc_hash(const Key& key) const {
		return (h(key) % (length-1));
	}
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
			auto& value = hash_entry::get(entry, key, visitor);
			current_size += entry->size() - before_size;
			return value;
		}
		else {
			visitor.entry_reset(entry, key);
			++current_size;
			return entry->getValue();
		}
	}

	bool insert(const Key& key, const Value& value) {
		const auto& hash = calc_hash(key);
		auto& entry = table[hash];
		if(entry) {
			const auto& inserted = hash_entry::insert(entry, key, value, visitor);
			if(inserted) {
				++current_size;
			}
			return inserted;
		}
		else {
			visitor.entry_reset(entry, key, value);
			++current_size;
			return true;
		}

	}     


	Value& operator[] (const Key& key) { return get(key); }
	//const Value& operator[] (const Key& key) const { return get(key); }

	bool erase(const Key& key) {
		const auto& hash = calc_hash(key);
		if (auto& entry = table[hash]) {
			const auto& removed = hash_entry::erase(entry, key, visitor);
			if( removed ) {
				--current_size;
			}
			return removed;
		}
		return false;
	}
	size_t size() const { return current_size; }



	HashTableIterator<HashTable> begin() { return HashTableIterator<HashTable>(*this, after(0)); }
	HashTableIterator<HashTable> end() { return HashTableIterator<HashTable>(*this,  nullptr); }

private:
		hash_entry_ptr& find(size_t h, size_t level) {
		for(auto hi = h; hi < length; ++hi) {
			if(auto& entry = table[hi]) {
				if(!entry.getNext()) { return entry; }
				size_t cl = 0;
				while(entry && cl != level) {
					entry = entry->getNext();
				}
				return entry;
			}
		}
		return empty;
	}

	hash_entry* after (size_t h) {
		for(auto hi = h+1; hi < length; ++hi) {
			if(auto& e = table[hi]) {
				return e.get();
			}
		}
		return nullptr;
	}

	hash_entry* next(hash_entry* entry) {
		if(!entry) {
			return nullptr;
		}
		if(auto& e = entry->getNext()) {
			return e.get();
		}
		const auto& h = calc_hash(entry->getKey());
		return after(h);
	}

	hash_entry_ptr& previos(hash_entry_ptr& entry) {
		if(!entry) {
			return nullptr;
		}
		const auto& h = calc_hash(entry->getKey());
		auto& e = table[h];
		if(e) {
			while(e) {
				auto& next = e->getNext();
				if(next && next->getKey() == entry->getKey()) {
					return e.get();
				}
				e = next;
			}
		}
		if(!h) {
			return nullptr;
		}
		if(h - 1 == 0) {
			auto& e1 = table[h-1];
			if(e1) {
				while(e1) {
					auto& next = e1->getNext();
					if(next && next->getKey() == entry->getKey()) {
						return e1.get();
					}
					e1 = next;
				}
			}
		}

		for(auto hi = h; hi != 0; --hi) {
			if(auto& e2 = table[hi]) {
				while(e2) {
					auto& next = e2->getNext();
					if(!next) {
						return e2;
					}
					e2 = next;
				}
				return e.get();
			}
		}
		return nullptr;
	}
};

template<class HTable>
HashTableIterator<HTable>& HashTableIterator<HTable>::operator--() {
	entry = table.previus(entry);
	return *this;
}

template<class HTable>
HashTableIterator<HTable> HashTableIterator<HTable>::operator--(int) {
	auto tmp = HashTableIterator<HTable>(table, entry);
	entry = table.previus(entry);
	return tmp;
}

template<class HTable>
HashTableIterator<HTable>& HashTableIterator<HTable>::operator++() {
	entry = table.next(entry);
	return *this;
}

template<class HTable>
HashTableIterator<HTable> HashTableIterator<HTable>::operator++(int) {
	auto tmp = HashTableIterator<HTable>(table, entry);
	entry = table.next(entry);
	return tmp;
}

template<class HTable>
typename HashTableIterator<HTable>::HEntry& HashTableIterator<HTable>::operator* () {
	if(!entry) {
		throw std::runtime_error("entry is empty");
	}
	return *entry;
}

#include <atomic>
#include <mutex>

template<class Key, class Value, class Mutex>
struct LockVisitor {
	typedef std::unique_lock<Mutex> lock;
	Mutex mx;
	typedef HashEntry<Key, Value, LockVisitor> hash_entry;
	template<class EntryPtr>
	void entry_reset (EntryPtr& entry, const Key& key, const Value& value) {
		entry.reset(new hash_entry(key, value));
	}
	template<class EntryPtr>
	void entry_reset (EntryPtr& entry, const Key& key) {
		entry.reset(new hash_entry(key, Value()));
	}
	template<class EntryPtr>
	void entry_set (EntryPtr& entry, const Value& value) {
		entry->getValue() = value;
	}
	template<class EntryPtr>
	void entry_move (EntryPtr& entry) {
		entry = std::move(entry->getNext());
	}
	lock read_lock() {
		return lock(mx);
	}
};

//TODO: shared_mutex, spinlock_mutex
template <class Key, class Value, class Mutex = std::mutex, class Visitor = LockVisitor<Key,Value,Mutex>>
class ConcurrencyHashTable
	: private HashTable<Key, Value, Visitor, std::atomic_size_t> {
	typedef HashTable<Key, Value, Visitor, std::atomic_size_t> base;
public:
	typedef typename base::hash_entry hash_entry;
	typedef Key key_type;
	typedef hash_entry value_type;
public:
		ConcurrencyHashTable() {
		}
		ConcurrencyHashTable(size_t capacity)
			: base(capacity) {

		}

		Value& get(const Key& key)  {
			auto lock = visitor.read_lock();
			return base::get(key);
		}

		bool insert(const Key& key, const Value& value) {
			auto lock = visitor.read_lock();
			return base::insert(key, value);
		}     

		bool erase(const Key& key) {
			auto lock = visitor.read_lock();
			return base::erase(key);
		}

		Value& operator[] (const Key& key) { return ConcurrencyHashTable::get(key); }
		//const Value& operator[] (const Key& key) const { return get(key); }


		size_t size() const { return base::size(); }

		friend typename hash_entry;
	
};

template <class Key, class Value, bool Sync = false>
class HashMap {
};

template <class Key, class Value>
class HashMap<Key,Value, true> 
	: public ConcurrencyHashTable<Key, Value>  {
	typedef ConcurrencyHashTable<Key, Value> base;
public:
	typedef typename base::hash_entry hash_entry;
	typedef Key key_type;
	typedef hash_entry value_type;

		HashMap() {
		}
		HashMap(size_t capacity)
			: base(capacity) {

		}
};

template<class Key, class Value>
struct NonlockVisitor {
	typedef HashEntry<Key, Value, NonlockVisitor> hash_entry;
	template<class EntryPtr>
	void entry_reset (EntryPtr& entry, const Key& key, const Value& value) {
		entry.reset(new hash_entry(key, value));
	}
	template<class EntryPtr>
	void entry_reset (EntryPtr& entry, const Key& key) {
		entry.reset(new hash_entry(key, Value()));
	}
	template<class EntryPtr>
	void entry_set (EntryPtr& entry, const Value& value) {
		entry->getValue() = value;
	}
	template<class EntryPtr>
	void entry_move (EntryPtr& entry) {
		entry = std::move(entry->getNext());
	}	
};

template <class Key, class Value>
class HashMap<Key,Value, false> 
	: public HashTable<Key, Value, NonlockVisitor<Key,Value>>  {
	typedef HashTable<Key, Value, NonlockVisitor<Key,Value>> base;
public:
	typedef typename base::hash_entry hash_entry;
	typedef Key key_type;
	typedef hash_entry value_type;

		HashMap() {
		}
		HashMap(size_t capacity)
			: base(capacity) {

		}

	
};
