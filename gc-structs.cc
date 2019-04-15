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
