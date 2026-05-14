#include <shivers.h>

unsigned long long get_timestamp() {
  #if defined(__linux__)
    struct timeval timestamp;
    gettimeofday(&timestamp, NULL);
    return ((timestamp.tv_sec * 1000) + (timestamp.tv_usec / 1000));
  #elif defined(_WIN32)
    struct _timeb timestamp;
    _ftime_s(&timestamp);
    return ((timestamp.time * 1000) + timestamp.millitm);
  #endif
}
