// String Builder

struct String_Builder {
	List<char> builder;
	String_Builder();
	~String_Builder();
	void append(const char * s);
	void append_size(const char * s, size_t size);
	size_t size();
	char at(size_t i);
	char * final_string();
};

String_Builder::String_Builder()
{
	builder.alloc();
}

String_Builder::~String_Builder()
{
	builder.dealloc();
}

void String_Builder::append(const char * s)
{
	for (int i = 0; i < strlen(s); i++) {
		builder.push(s[i]);
	}
}

void String_Builder::append_size(const char * s, size_t size)
{
	for (int i = 0; i < size; i++) {
		builder.push(s[i]);
	}
}

char * String_Builder::final_string()
{
	char * str = (char*) malloc(sizeof(char) * (builder.size + 1));
	strncpy(str, builder.arr, builder.size);
	str[builder.size] = '\0';
	return str;
}

size_t String_Builder::size()
{
	return builder.size;
}

char String_Builder::at(size_t i)
{
	assert(i < builder.size);
	return builder[i];
}

//
