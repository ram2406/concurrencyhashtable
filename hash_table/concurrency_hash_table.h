
template <class Key, class Value, class Mutex = std::mutex, class Visitor = LockVisitor<Key,Value,Mutex>>
class ConcurrencyHashTable
	: private HashTable<Key, Value, Visitor, std::atomic_size_t> {
	typedef HashTable<Key, Value, Visitor, std::atomic_size_t> base;
public:
	typedef typename base::hash_entry hash_entry;
	typedef Key key_type;
	typedef hash_entry value_type;
	typedef typename Visitor::lock lock_type;
	typedef Mutex mutex_type;
	friend class HashTableIterator<ConcurrencyHashTable>;
public:
		ConcurrencyHashTable() {
		}
		ConcurrencyHashTable(size_t capacity)
			: base(capacity) {

		}

		Value get(const Key& key)  {
			auto hash = calc_hash(key);
			auto lock = visitor.read_lock(hash);
			return base::get(key, hash);
		}

		bool insert(const Key& key, const Value& value) {
			auto hash = calc_hash(key);
			auto lock = visitor.read_lock(hash);
			return base::insert(key, value, hash);
		}     

		bool erase(const Key& key) {
			auto hash = calc_hash(key);
			auto lock = visitor.read_lock(hash);
			return base::erase(key, hash);
		}

		Value operator[] (const Key& key) { return ConcurrencyHashTable::get(key); }
		//const Value& operator[] (const Key& key) const { return get(key); }


		size_t size() const { return base::size(); }

		HashTableIterator<ConcurrencyHashTable> begin() { 
			
			auto ep = ConcurrencyHashTable::after(0);
			return ep 
				? HashTableIterator<ConcurrencyHashTable>(*this, ep->getKey())
				: end();
		}
		HashTableIterator<ConcurrencyHashTable> end() { return HashTableIterator<ConcurrencyHashTable>(*this); }

private:
	hash_entry* next(const Key& key) {
		const auto& hash = calc_hash(key);
		auto lock = visitor.read_lock(hash);
		return base::next_by_hash(hash);
	}

	hash_entry* previos(const Key& key) {
		const auto& hash = calc_hash(key);
		auto lock = visitor.read_lock(hash);
		return base::previos_by_hash(hash);
	}
	hash_entry* after(size_t h) {
		for (auto hi = h + 1; hi < getLength(); ++hi) {
			auto lock = visitor.read_lock(hi);
			if (auto& e = getTable()[hi]) {
				return e.get();
			}
		}
		return nullptr;
	}
};
