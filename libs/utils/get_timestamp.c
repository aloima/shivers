#include <stddef.h>

#include <sys/time.h>

unsigned long get_timestamp(const struct timeval *val) {
	if (val == NULL) {
		struct timeval timestamp;
		gettimeofday(&timestamp, NULL);
		return ((timestamp.tv_sec * 1000) + (timestamp.tv_usec / 1000));
	} else {
		const struct timeval *timestamp = val;
		return ((timestamp->tv_sec * 1000) + (timestamp->tv_usec / 1000));
	}
}
