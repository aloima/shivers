#include <stdbool.h>

#include <sys/time.h>

#ifndef UTIL_H_
	#define UTIL_H_

	struct SplitData {
		char *data;
		unsigned long length;
	};

	struct Split {
		struct SplitData *data;
		unsigned int size;
	};

	struct Join {
		char *data;
		unsigned long length;
	};

	struct Split split(const char *text, const unsigned long length, const char *separator);
	void split_free(struct Split value);

	void *allocate(void *value, const unsigned long old_count, const unsigned long new_count, const unsigned char size);

	unsigned long join(const struct Join *value, char *source, unsigned short size, const char *separator);
	unsigned long calculate_join(const struct Join *value, unsigned short size, const char *separator);
	void create_join_elements_nz(struct Join *joins, const char **list, const unsigned int list_size);

	void strtolower(char *source, const char *dest);
	void strtoupper(char *source, const char *dest);
	bool strsame(const char *str1, const char *str2);
	void strreplace(char **source, char *target, char *replacement);
	const char *ltrim(const char *src);

	unsigned long ahtoi(const char *data);
	char *base64_encode(const char *data, const unsigned long data_length);

	long char_at(const char *data, const char ch);

	unsigned long long get_timestamp();

	void throw(const char *format, ...);

	void sort(int *data, const unsigned long size);
#endif
