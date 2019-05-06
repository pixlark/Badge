struct Assoc {
	const char * source;
	size_t source_length;
	size_t position;
	size_t extents;
};

typedef int Assoc_Ptr;

namespace Assoc_Allocator {
	List<Assoc> assocs;
	void init()
	{
		assocs.alloc();
	}
	void destroy()
	{
		assocs.dealloc();
	}
	Assoc_Ptr make(const char * source, size_t source_length, size_t position, size_t extents)
	{
		assocs.push((Assoc) { source, source_length, position, extents });
		return assocs.size - 1;
	}
	Assoc get(Assoc_Ptr pointer)
	{
		return assocs[pointer];
	}
	Assoc * ptr(Assoc_Ptr pointer)
	{
		return &assocs[pointer];
	}
};

#define RESET         "\e[0m"
#define SET_BOLD      "\e[1m"
#define SET_DIM       "\e[2m"
#define SET_UNDERLINE "\e[4m"
#define SET_BLINK     "\e[5m"
#define SET_INVERTED  "\e[7m"
#define SET_HIDDEN    "\e[8m"

#define SET_RED "\e[31m"

#define BOLD(x) SET_BOLD x RESET
#define DIM(x) SET_DIM x RESET
#define INVERTED(x) SET_INVERTED x RESET
#define RED(x)  SET_RED x RESET

void error(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, RED(BOLD("encountered error")) ":\n");
	vfprintf(stderr, fmt, args);
	printf("\n");

	va_end(args);
	abort();
}

void fatal(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, RED(BOLD("encountered error")) ":\n");
	vfprintf(stderr, fmt, args);
	printf("\n");

	va_end(args);
	//exit(1);
	abort();
}

void print_assoc(Assoc assoc)
{
	size_t start = assoc.position;
	while (true) {
		if (start == 0) {
			break;
		}
		if (assoc.source[start - 1] == '\n') {
			break;
		}
		start--;
	}
	size_t end = assoc.position;
	while (true) {
		if (end == assoc.source_length - 1) {
			break;
		}
		if (assoc.source[end + 1] == '\n') {
			break;
		}
		end++;
	}
	// Line itself
	const char * selection = assoc.source + start;
	size_t selection_length = end - start + 1;
	printf(DIM("  %.*s\n"), (int) selection_length, selection);
	// Pointer char
	printf("  ");
	for (int i = 0; i < assoc.position - start; i++) {
		if (selection[i] == '\t') {
			printf("\t");
		} else {
			printf(" ");
		}
	}
	for (int i = 0; i < assoc.extents; i++) {
		printf(RED("^"));
	}
	printf("\n");
}

void v_fatal_assoc(Assoc_Ptr assoc, const char * fmt, va_list args)
{
	fprintf(stderr, RED(BOLD("encountered error")) ":\n");
	vfprintf(stderr, fmt, args);
	printf("\n");
	if (assoc != -1) {
		print_assoc(Assoc_Allocator::get(assoc));
	}
	abort();
}

void fatal_assoc(Assoc_Ptr assoc, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	v_fatal_assoc(assoc, fmt, args);
	
	va_end(args);
}

/** TODO(pixlark): Do this without GCC dependency... like so? https://stackoverflow.com/questions/18777843/variadic-macro-with-empty-argument-in-c11

void _fatal_internal(const char * fmt, const char * file, size_t line, ...)
{
	va_list args;
	va_start(args, line);

	fprintf(stderr, INVERTED(RED(BOLD("internal error"))) ":\n");
	fprintf(stderr, DIM("%s:%zu") "\n", file, line);
	vfprintf(stderr, fmt, args);
	printf("\n");

	va_end(args);
	abort();
}

// TODO(pixlark): GCC-dependent... not really much of a choice though
#define fatal_internal(fmt, ...) _fatal_internal(fmt, __FILE__, __LINE__, ##__VA_ARGS__)

//

*/
