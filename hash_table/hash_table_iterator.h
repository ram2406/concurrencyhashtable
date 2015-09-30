
template<class HTable>
class HashTableIterator : public std::iterator<std::output_iterator_tag, typename HTable::hash_entry>
{
	typedef typename HTable::hash_entry HEntry;
	typedef typename HTable::key_type Key;
	HTable& table;
	Key key;
	bool has_value;
public:
	HashTableIterator(HTable& table, const Key &key) : table(table), key(key), has_value(true) {}
	HashTableIterator(HTable& table) : table(table), has_value(false) {}
  HashTableIterator(const HashTableIterator& it) : table(it.table), key(it.key), has_value(true) {}

  HashTableIterator& operator--();
  HashTableIterator operator--(int);

  HashTableIterator& operator++();
  HashTableIterator operator++(int);

  HEntry& operator*();

  bool operator==(const HashTableIterator& rhs) { return has_value == rhs.has_value && &table == &rhs.table && (!has_value || key == rhs.key); }
  bool operator!=(const HashTableIterator& rhs) { return !this->operator==(rhs); }

  
};
