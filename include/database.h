#include <json.h>

#ifndef DATABASE_H_
	#define DATABASE_H_

	void database_initialize(const char *filename);
	void database_save();
	void database_destroy();

	void database_set(char *key, void *value, const enum JSONType type);
	void database_push(char *key, void *value, const enum JSONType type);
	void database_delete(const char *key);
	jsonresult_t database_get(const char *key);
#endif
