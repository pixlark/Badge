struct Blocks {
	List<BC*> blocks;
	List<size_t> sizes;
	void init()
	{
		blocks.alloc();
		sizes.alloc();
	}
	size_t make_block()
	{
		blocks.push(NULL);
		sizes.push(0);
		return blocks.size - 1;
	}
	void finalize_block(size_t reference, BC * block, size_t size)
	{
		blocks[reference] = block;
		sizes[reference] = size;
	}
	size_t size_block(size_t reference)
	{
		return sizes[reference];
	}
	BC * retrieve_block(size_t reference)
	{
		assert(blocks[reference]);
		return blocks[reference];
	}
	void destroy()
	{
		for (int i = 0; i < blocks.size; i++) {
			free(blocks[i]);
		}
		blocks.dealloc();
		sizes.dealloc();
	}
};
