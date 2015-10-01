
template<class Key, class Value>
struct NonlockVisitor {
	typedef HashEntry<Key, Value, NonlockVisitor> hash_entry;
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
};
