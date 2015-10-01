

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

	const size_t& getLength() const { return length; }
	const std::vector<std::unique_ptr<hash_entry>>& getTable() const { return table; }

	size_t calc_hash(const Key& key) const {
		return (h(key) % (length-1));
	}
	hash_entry_ptr& entry_of(const Key& key) {
		return table[calc_hash(key)];
	}

	hash_entry* after (size_t h) {
		for(auto hi = h+1; hi < length; ++hi) {
			if(auto& e = table[hi]) {
				return e.get();
			}
		}
		return nullptr;
	}
	hash_entry* next(const Key& key) {
		const auto& hash = calc_hash(key);
		return next_by_hash(hash);
	}
	hash_entry* next_by_hash(size_t hash) {
		return next(table[hash].get());
	}

	hash_entry* previos(const Key& key) {
		const auto& hash = calc_hash(key);
		return previos_by_hash(hash);
	}

	hash_entry* previos_by_hash(size_t hash) {
		return previos(table[hash].get());
	}

public:
	HashTable()
		: table(DefaultCapacity), length(DefaultCapacity), current_size(0) {

	}
	HashTable(size_t capacity)
		: table(capacity), length(capacity), current_size(0) {

	}

	Value& get(const Key& key, size_t hash) {
		if (auto& entry = table[hash]) {
			const auto& before_size = entry->size();
			auto& value = hash_entry::get(entry, key, hash, visitor);
			current_size += entry->size() - before_size;
			return value;
		}
		else {
			visitor.entry_reset(entry, key, hash);
			++current_size;
			return entry->getValue();
		}
	}

	Value& get(const Key& key)  {
		auto hash = calc_hash(key);
		return get(key, hash);
	}

	bool insert(const Key& key, const Value& value, size_t hash) {
		auto& entry = table[hash];
		if (entry) {
			const auto& inserted = hash_entry::insert(entry, key, value, hash, visitor);
			if (inserted) {
				++current_size;
			}
			return inserted;
		}
		else {
			visitor.entry_reset_value(entry, key, value, hash);
			++current_size;
			return true;
		}
	}

	bool insert(const Key& key, const Value& value) {
		auto hash = calc_hash(key);
		return insert(key, value, hash);
	}     


	Value& operator[] (const Key& key) { return get(key); }
	//const Value& operator[] (const Key& key) const { return get(key); }
	bool erase(const Key& key, size_t hash) {
		if (auto& entry = table[hash]) {
			const auto& removed = hash_entry::erase(entry, key, hash, visitor);
			if (removed) {
				--current_size;
			}
			return removed;
		}
		return false;
	}
	bool erase(const Key& key) {
		auto hash = calc_hash(key);
		return erase(key, hash);
	}
	size_t size() const { return current_size; }



	HashTableIterator<HashTable> begin() { 
		auto ep = after(0); 
		return ep 
			? HashTableIterator<HashTable>(*this, ep->getKey())
			: end();
	}
	HashTableIterator<HashTable> end() { return HashTableIterator<HashTable>(*this); }

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

	hash_entry* previos(hash_entry* entry) {
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