#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include <json.h>
#include <utils.h>

static jsonelement_t *data = NULL;
static char *_filename = NULL;

void database_initialize(const char *filename) {
	_filename = (char *) filename;

	struct stat file_entry;
	const char file_stat = stat(filename, &file_entry);

	if (file_stat == -1) {
		const char empty[3] = "{}";

		FILE *file = fopen(filename, "w");
		fwrite(empty, sizeof(char), 2, file);
		fclose(file);

		data = json_parse(empty);
	} else {
		FILE *file = fopen(filename, "r");
		fseek(file, 0, SEEK_END);
		const unsigned long long size = ftell(file);
		rewind(file);

		char content[size + 1];
		fread(content, sizeof(char), size, file);
		content[size] = '\0';
		fclose(file);

		data = json_parse(content);
	}
}

void database_save() {
	char *content = json_stringify(data, 5);
	FILE *file = fopen(_filename, "w");
	fwrite(content, sizeof(char), strlen(content), file);
	fclose(file);
	free(content);
}

void database_destroy() {
	json_free(data, false);
}

void database_set(char *key, void *value, const unsigned char type) {
	json_set_val(data, key, value, type);
}

bool database_has(const char *key) {
	return json_get_val(data, key).exist;
}

void database_delete(const char *key) {
	json_del_val(data, key);
}

jsonvalue_t database_get(const char *key) {
	return json_get_val(data, key).value;
}
