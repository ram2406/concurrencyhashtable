
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

		Value operator[] (const Key& key) { return ConcurrencyHashTable::get(key); }
		//const Value& operator[] (const Key& key) const { return get(key); }


		size_t size() const { return base::size(); }

		HashTableIterator<ConcurrencyHashTable> begin() { 
			auto lock = visitor.read_lock();
			auto ep = base::after(0);
			return ep 
				? HashTableIterator<ConcurrencyHashTable>(*this, ep->getKey())
				: end();
		}
		HashTableIterator<ConcurrencyHashTable> end() { return HashTableIterator<ConcurrencyHashTable>(*this); }

private:
	hash_entry* next(const Key& key) {
		auto lock = visitor.read_lock();
		return base::next(key);
	}

	hash_entry* previos(const Key& key) {
		auto lock = visitor.read_lock();
		return base::previos(key);
	}

};
