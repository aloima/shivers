#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include <database.h>
#include <utils.h>
#include <json.h>

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
		const unsigned int size = ftell(file);
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

void database_push(char *key, void *value, const unsigned char type) {
	jsonresult_t array = json_get_val(data, key);

	if (array.exist) {
		char json_key[(array.element->size / 10) + 4];
		sprintf(json_key, "[%ld]", array.element->size);

		json_set_val(array.element, json_key, value, type);
	} else {
		char json_key[5 + strlen(key)];
		sprintf(json_key, "%s.[0]", key);

		json_set_val(data, json_key, value, type);
	}
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
