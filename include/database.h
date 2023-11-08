#include <json.h>

#ifndef DATABASE_H_
	#define DATABASE_H_

	void database_initialize(const char *filename);
	void database_save();
	void database_destroy();

	void database_set(char *key, void *value, const unsigned char type);
	bool database_has(const char *key);
	jsonvalue_t database_get(const char *key);
#endif
