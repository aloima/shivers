#include <string.h>

#include <sys/time.h>

#ifndef UTIL_H_
	#define UTIL_H_

	struct SplitData {
		char *data;
		size_t length;
	};

	struct Split {
		struct SplitData *data;
		size_t size;
	};

	struct Join {
		char *data;
		size_t length;
	};

	struct Split split(const char *text, const size_t length, const char *separator);
	void split_free(struct Split value);

	void *allocate(void *value, const long old_count, const long new_count, const unsigned char size);

	size_t join(const struct Join *value, char *source, unsigned short size, const char *separator);
	size_t calculate_join(const struct Join *value, unsigned short size, const char *separator);
	void create_join_elements_nz(struct Join *joins, const char **list, const size_t list_size);

	void strtolower(char *source, const char *dest);
	void strtoupper(char *source, const char *dest);
	const char *ltrim(const char *src);

	size_t ahtoi(const char *data);
	char *base64_encode(const char *data, const size_t data_length);

	long char_at(const char *data, const char ch);

	unsigned long get_timestamp(const struct timeval *val);

	void throw(const char *format, ...);

	void sort(long *data, const size_t size);
#endif
