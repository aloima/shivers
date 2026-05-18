#pragma once

#include <stdint.h>

// To guarantee code execution
#define ASSERT(actual, op, expected) ({         \
  typeof(actual) __actual_val = (actual);       \
  typeof(expected) __expected_val = (expected); \
  assert(__actual_val op __expected_val);       \
  __actual_val;                                 \
})

#define SPRINTF_S(__s, __format, ...) ASSERT(sprintf((__s), (__format), __VA_ARGS__), >, 0)

struct SplitData {
  char *data;
  unsigned int length;
};

struct Sort {
  void *value;
  int number;
};

struct String {
  char *value;
  unsigned int length;
};

struct Split {
  struct SplitData *data;
  unsigned int size;
};

typedef struct Join {
  char *data;
  unsigned int length;
} Join;

struct Split split(const char *text, const unsigned int length, const char *separator);
void split_free(struct Split value);

void *allocate(void *value, const long old_count, const unsigned long new_count, const unsigned char size);

uint64_t join(const Join *value, char *source, uint16_t size, const char *separator);
uint64_t calculate_join(const Join *value, uint16_t size, const char *separator);

void strtolower(char *source, const char *dest);
void strtoupper(char *source, const char *dest);
bool streq(const char *str1, const char *str2);
void strreplace(char **source, char *target, char *replacement);
char *ltrim(const char *src);

// Converts hex string to integer, -1 on failure
int64_t ahtoi(const char *data);

// Converts string to integer safely -1 on failure
int64_t atoi_s(const char *str, int16_t length);


char *base64_encode(const char *data, const unsigned long data_length);
#define BASE64_ENCODE_ERROR ("base64_encode(): cannot decode key")

int char_at(char *data, const char ch);

int64_t get_timestamp();
#define GET_TIMESTAMP_ERROR ("get_timestamp(): Cannot get current time")

void throw(const char *format, ...);

void sort(struct Sort *data, const unsigned int size);
