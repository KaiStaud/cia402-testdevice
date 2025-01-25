#include <sys/time.h>
#include <time.h>

int
_gettimeofday(struct timeval *tp, void *tzp)
{
	(void)tzp;

	if (tp) {
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		*tp = (struct timeval){ ts.tv_sec, ts.tv_nsec / 1000 };

	}
	return 0;
}
