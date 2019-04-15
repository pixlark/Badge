typedef void *(*Malloc)(size_t);
typedef void (*Free)(void*);

struct Allocator {
	Malloc __malloc;
	Free __free;
	static Allocator construct(Malloc __malloc, Free __free)
	{
		Allocator allocator;
		allocator.__malloc = __malloc;
		allocator.__free = __free;
		return allocator;
	}
};

static Allocator default_allocator = Allocator::construct(malloc, free);
