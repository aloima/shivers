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

	void *allocate(void *value, const long old_count, const long new_count, const unsigned char size);

	size_t join(char **value, char *source, unsigned short size, const char *separator);
	size_t calculate_join(char **value, unsigned short size, const char *separator);

	void strtolower(char *source, const char *dest);
	void strtoupper(char *source, const char *dest);
	const char *ltrim(const char *src);

	size_t ahtoi(const char *data);
	char *base64_encode(const char *data, const size_t data_length);

	size_t char_at(const char *data, const char ch);

	unsigned long get_timestamp(const struct timeval *val);

	void throw(const char *format, ...);

	void sort(long *data, const size_t size);
#endif
