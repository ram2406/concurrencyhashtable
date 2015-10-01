
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
		: key(key), value(value) {
	}

	const Key& getKey() const { return key; }
	Value& getValue() { return value; }
	HashEntryPtr& getNext() { return next; }

	bool insert(const Key& key, const Value& value, size_t hash, Visitor& v) {
		if(this->getKey() == key) {
			v.entry_set(this, value, hash);
			return false;
		}
		if(!next) {
			v.entry_reset_value(next, key, value, hash);
			return true;
		}
		return next->insert(key, value, hash, v);
	}

	// top function for call recursive insert
	// return true if element inserted, or false if changed
	static bool insert(HashEntryPtr& entry, const Key& key, const Value& value, size_t hash, Visitor& v) {
		if(entry->getKey() == key) {
			v.entry_set(entry, value, hash);
			return false;
		}
		if(!entry) {
			v.entry_reset_value(entry, key, value, hash);
			return true;
		}
		return entry->insert(key, value, hash, v);
	}

	bool erase(const Key& key, size_t hash, Visitor& v) {
		if(!next) {
			return false;
		}
		if(next->getKey() == key) {
			v.entry_move(next, hash);
			return true;
		}
		return next->erase(key, hash, v);
	}

	// top function for call recursive erase
	// return true if element finded by key and removed
	static bool erase(HashEntryPtr& entry, const Key& key, size_t hash, Visitor& v) {
		if(entry->getKey() == key) {
			v.entry_move(entry, hash);
			return true;
		}
		return entry->erase(key, hash, v);
	}
	//calc elements count of linked list
	size_t size() const {
		if(!next) {
			return 0;
		}
		return 1 + next->size();
	}

	// top function for call recursive get
	static Value& get(HashEntryPtr& entry, const Key& key, size_t hash, Visitor& v) {
		if(entry->getKey() == key) {
			return entry->getValue();
		}
		return entry->get(key, hash, v);
	}

	Value& get(const Key& key, size_t hash, Visitor& v) {
		if(this->getKey() == key) {
			return this->getValue();
		}
		if(!next) {
			v.entry_reset(next, key, hash);
			return next->getValue();
		}
		return next->get(key, hash, v);
	}

};

