#include <string.h>

#include <sys/time.h>

#ifndef UTIL_H_
	#define UTIL_H_

	typedef struct {
		char **data;
		size_t size;
	} Split;

	Split split(const char *text, const char *separator);
	void split_free(Split *value);

	void *allocate(void *value, const size_t old_count, const size_t new_count, const size_t size);

	size_t join(char **value, char *source, size_t size, char *separator);
	size_t calculate_join(char **value, size_t size, char *separator);

	void strtolower(char *source, const char *dest);
	void strtoupper(char *source, const char *dest);
	const char *ltrim(const char *src);

	size_t ahtoi(const char *data);
	char *base64_encode(const char *data, const size_t data_length);

	const size_t char_at(const char *data, const char ch);

	const unsigned long get_timestamp(struct timeval *val);

	void throw(const char *format, ...);
#endif
