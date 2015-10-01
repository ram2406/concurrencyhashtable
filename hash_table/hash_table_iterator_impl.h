
template<class HTable>
HashTableIterator<HTable>& HashTableIterator<HTable>::operator--() {
	if(!has_value) {
		throw std::runtime_error("invalid iterator");
	}
	auto entry = table.previus(key);
	if(!entry) {
		has_value = false;
	}
	else {
		key = entry->getKey();
	}
	return *this;
}

template<class HTable>
HashTableIterator<HTable> HashTableIterator<HTable>::operator--(int) {
	if(!has_value) {
		throw std::runtime_error("invalid iterator");
	}
	auto tmp = HashTableIterator<HTable>(table, entry);
	operator--();
	return tmp;
}

template<class HTable>
HashTableIterator<HTable>& HashTableIterator<HTable>::operator++() {
	if(!has_value) {
		throw std::runtime_error("invalid iterator");
	}
	auto entry = table.next(key);
	if(!entry) {
		has_value = false;
	}
	else {
		key = entry->getKey();
	}
	return *this;
}

template<class HTable>
HashTableIterator<HTable> HashTableIterator<HTable>::operator++(int) {
	if(!has_value) {
		throw std::runtime_error("invalid iterator");
	}
	auto tmp = HashTableIterator<HTable>(table, key);
	operator++();
	return tmp;
}

template<class HTable>
typename HashTableIterator<HTable>::HEntry& HashTableIterator<HTable>::operator* () {
	typedef typename HashTableIterator<HTable>::HEntry hentry;
	if(!has_value) {
		throw std::runtime_error("invalid iterator");
	}
	
	return *table.entry_of(key);
}
