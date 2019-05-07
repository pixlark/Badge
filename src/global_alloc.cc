/* We want to be able to check for memory leaks after the program has
 * ended. This means we need to clean up literally everything we
 * allocate. But if we want something to be allocated and just live
 * forever, that makes things difficult. So we just do that through
 * this global allocator that will clean up before program exit.
 *
 */

namespace Global_Alloc {
	List<void*> allocations;
	void init()
	{
		allocations.alloc();
	}
	void destroy()
	{
		for (int i = 0; i < allocations.size; i++) {
			free(allocations[i]);
		}
		allocations.dealloc();
	}
	void * alloc(size_t size)
	{
		allocations.push(malloc(size));
		return allocations[allocations.size - 1];
	}
}
