template <typename T>
struct GC_List : List<T> {
	void alloc()
	{
		List<T>::alloc(GC::allocator);
	}
	void gc_mark()
	{
		GC::mark_opaque(List<T>::get_raw());
	}
};

template <typename K, typename V>
struct GC_Map : Map<K, V> {
	void alloc(bool (*comparator)(K, K))
	{
		Map<K, V>::alloc(comparator, GC::allocator);
	}
	void gc_mark()
	{
		GC::mark_opaque(Map<K, V>::get_keys()->get_raw());
		GC::mark_opaque(Map<K, V>::get_values()->get_raw());
	}
};
