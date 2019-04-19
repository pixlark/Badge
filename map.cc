template <typename K, typename V>
// TODO(pixlark): Make this less shite
struct Map {
	bool (*comparator)(K, K);
	List<K> keys;
	List<V> values;

	// fuck off C++
	List<K> * get_keys();
	List<V> * get_values();
	
	void alloc(bool (*comparator)(K, K), Allocator allocator = default_allocator);
	bool bound(K key);
	bool add(K key, V value);
	bool update(K key, V value);
	V lookup(K key);
};

template <typename K, typename V>
List<K> * Map<K, V>::get_keys()
{
	return &keys;
}

template <typename K, typename V>
List<V> * Map<K, V>::get_values()
{
	return &values;
}

template <typename K, typename V>
void Map<K, V>::alloc(bool (*comparator)(K, K), Allocator allocator)
{
	this->comparator = comparator;
	keys.alloc(allocator);
	values.alloc(allocator);
}

template <typename K, typename V>
bool Map<K, V>::bound(K key)
{
	for (int i = 0; i < keys.size; i++) {
		if (comparator(keys[i], key)) {
			return true;
		}
	}
	return false;
}

template <typename K, typename V>
bool Map<K, V>::add(K key, V value)
{
	if (bound(key)) {
		return false;
	}
	keys.push(key);
	values.push(value);
	return true;
}

template <typename K, typename V>
bool Map<K, V>::update(K key, V value)
{
	for (int i = 0; i < keys.size; i++) {
		if (comparator(keys[i], key)) {
			values[i] = value;
			return true;
		}
	}
	return false;
}

// Will actually just die if it's not bound -- always check `bound()`
// first!
template <typename K, typename V>
V Map<K, V>::lookup(K key)
{
	for (int i = 0; i < keys.size; i++) {
		if (comparator(keys[i], key)) {
			return values[i];
		}
	}
	assert("Not bound in Map!" && 0);
}
