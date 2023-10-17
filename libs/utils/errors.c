#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void throw(const char *format, ...) {
	char message[256];
	va_list args;
	va_start(args, format);
	vsnprintf(message, 256, format, args);
	va_end(args);

	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}
