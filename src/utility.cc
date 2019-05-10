char * load_string_from_file(const char * path)
{
	FILE * file = fopen(path, "r");
	if (file == NULL) return NULL;
	defer { fclose(file); };
	
	int file_len = 0;
	while (fgetc(file) != EOF) file_len++;
	char * str = (char*) Global_Alloc::alloc(file_len + 1);
	str[file_len] = '\0';
	fseek(file, 0, SEEK_SET);
	for (int i = 0; i < file_len; i++) str[i] = fgetc(file);

	return str;
}

char * itoa(int integer)
{
	size_t size = snprintf(NULL, 0, "%d", integer);
	char * buf = (char*) malloc(sizeof(char) * (size + 1));
	sprintf(buf, "%d", integer);
	return buf;
}

bool int_comparator(int a, int b) { return a == b; }

