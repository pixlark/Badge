#define COLLECTION true
#define SHOW_COLLECTIONS true

namespace GC {
	List<void*> allocations;
	
	void init()
	{
		allocations.alloc();
	}
	void destroy()
	{
		allocations.dealloc();
	}
	void * raw_to_opaque(void * ptr)
	{
		return (void*) (((uint8_t*) ptr) + 1);
	}
	void * opaque_to_raw(void * ptr)
	{
		return (void*) (((uint8_t*) ptr) - 1);
	}
	void * alloc(size_t size)
	{
		void * ptr = malloc(size + 1);
		allocations.push(ptr);
		return raw_to_opaque(ptr);
	}
	void release(void * ptr)
	{
		// do nothing!
	}
	void mark_opaque(void * ptr)
	{
		*((uint8_t*) opaque_to_raw(ptr)) = 1;
	}
	bool is_marked_opaque(void * ptr)
	{
		return *((uint8_t*) opaque_to_raw(ptr));
	}
	void unmark_all()
	{
		for (int i = 0; i < allocations.size; i++) {
			*((uint8_t*) allocations[i]) = 0;
		}
	}
	void free_unmarked()
	{
		List<void*> new_allocations;
		new_allocations.alloc();
		for (int i = 0; i < allocations.size; i++) {
			if (*((uint8_t*) allocations[i])) {
				new_allocations.push(allocations[i]);
			} else {
				free(allocations[i]);
			}
		}
		#if SHOW_COLLECTIONS
		printf("\n-- %zu collections\n\n", allocations.size - new_allocations.size);
		#endif
		allocations.dealloc();
		allocations = new_allocations;
	}

	Allocator allocator = Allocator::construct(alloc, release);
}
