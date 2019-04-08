enum BC_Kind {
	BC_LOAD_CONST,
	BC_CREATE_BINDING,
	BC_UPDATE_BINDING,
	BC_POP_AND_PRINT,
};

struct BC {
	BC_Kind kind;
	Value arg;
	static BC create(BC_Kind kind)
	{
		BC bc;
		bc.kind = kind;
		return bc;
	}
	static BC create(BC_Kind kind, Value arg)
	{
		BC bc;
		bc.kind = kind;
		bc.arg = arg;
		return bc;
	}
};
