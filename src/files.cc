namespace Files {
	const size_t path_max = PATH_MAX;
	const char * first_file;
	const char * cwd;
	const char * running_dir;
	const char * stdlib_dir;
	void init(const char * first_file)
	{
		Files::first_file = strdup(first_file);
		
		{ // set cwd
			char buf[path_max];
			getcwd(buf, path_max);

			String_Builder builder;
			builder.append(buf);
			builder.append("/");
			cwd = builder.final_string();
		}
		{ // set running_dir
			size_t i = strlen(first_file);
			while (i > 0 && first_file[i - 1] != '/') {
				i--;
			}
			
			String_Builder builder;
			builder.append(cwd);
			if (i == 0) {
				// @Hack kind of a hack
				builder.append("./");
			} else {
				builder.append_size(first_file, i);
			}
			
			running_dir = builder.final_string();
		}
		{ // set stdlib dir
			String_Builder builder;
			auto env_variable = getenv("BADGE_STDLIB_PATH");
			if (!env_variable) {
				fatal("$BADGE_STDLIB_PATH not set!");
			}
			builder.append(env_variable);
			if (builder.at(builder.size() - 1) != '/') {
				builder.append("/");
			}
			stdlib_dir = builder.final_string();
		}
	}
	void destroy()
	{
		free((void*) running_dir);
		free((void*) cwd);
		free((void*) first_file);
	}
	const char * stdlib_file(const char * name)
	{
		String_Builder builder;
		builder.append(stdlib_dir);
		builder.append(name);
		builder.append(".bdg");
		return builder.final_string();
	}
	const char * path_for_file(const char * filename)
	{
		String_Builder builder;
		builder.append(running_dir);
		builder.append(filename);
		return builder.final_string();
	}
}
