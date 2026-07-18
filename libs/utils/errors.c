#include <shivers.h>

void throw(const char *format, ...) {
  char message[256];
  va_list args;
  va_start(args, format);
  ASSERT(vsnprintf(message, 256, format, args), >, 0);
  va_end(args);

  ASSERT(fprintf(stderr, "%s\n", message), >, 0);
  exit(EXIT_FAILURE);
}
