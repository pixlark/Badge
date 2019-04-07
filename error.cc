// Error handling

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
