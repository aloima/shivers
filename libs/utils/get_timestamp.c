#include <stddef.h>

#include <sys/time.h>

const unsigned long get_timestamp(struct timeval *val) {
	if (val == NULL) {
		struct timeval timestamp;
		gettimeofday(&timestamp, NULL);
		return ((timestamp.tv_sec * 1000) + (timestamp.tv_usec / 1000));
	} else {
		struct timeval *timestamp = val;
		return ((timestamp->tv_sec * 1000) + (timestamp->tv_usec / 1000));
	}
}
