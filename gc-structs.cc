/* GC-STRUCTS 
 * These are structures that are garbage collected, but aren't
 * directly used in the language; i.e., not Values but rather
 * conveniences for the developer.
 */

template <typename T>
struct GC_List {
	T * arr;
	size_t size;
	size_t capacity;
	
	void make()
	{
		size = 0;
		capacity = 4;
		arr = (T*) GC::alloc(sizeof(T) * capacity);
	}
	void gc_mark()
	{
		GC::mark_opaque(arr);
	}
	void grow()
	{
		capacity *= 2;
		T * narr = (T*) GC::alloc(sizeof(T) * capacity);
		memcpy(narr, arr, sizeof(T) * size);
		arr = narr;
	}
	void shrink()
	{
		capacity /= 2;
		T * narr = (T*) GC::alloc(sizeof(T) * capacity);
		memcpy(narr, arr, sizeof(T) * size);
		arr = narr;
	}
	void push(T item)
	{
		if (size + 1 > capacity) {
			grow();
		}
		arr[size++] = item;
	}
	T pop()
	{
		if (size - 1 < capacity / 4) {
			shrink();
		}
		return arr[--size];
	}
	T& operator[](size_t index)
	{
		assert(index < size);
		return arr[index];
	}
};
