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

#include "hash_entry.h"
#include "hash_table_iterator.h"
#include "hash_table.h"
#include "hash_table_iterator_impl.h"

#include "lock_visitor.h"
#include "concurrency_hash_table.h"

#include "non_lock_visitor.h"

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
