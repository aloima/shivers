#include <shivers.h>

int64_t get_timestamp() {
  #if defined(__linux__)
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
      return -1;

    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000);
  #elif defined(__WIN32__)
    struct _timeb timestamp;
    if (_ftime_s(&timestamp) != 0)
      return -1;

    return ((timestamp.time * 1000) + timestamp.millitm);
  #endif
}
