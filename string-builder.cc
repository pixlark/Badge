// String Builder

struct String_Builder {
	List<char> builder;
	String_Builder();
	~String_Builder();
	void append(const char * s);
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

char * String_Builder::final_string()
{
	char * str = (char*) malloc(sizeof(char) * (builder.size + 1));
	strncpy(str, builder.arr, builder.size);
	str[builder.size] = '\0';
	return str;
}

//
