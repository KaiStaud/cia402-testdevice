#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

clock_t
_times(struct tms *buffer)
{
	// Calculate the number of ticks from the time-of-day.
	struct timeval tod = { 0, 0 };
	gettimeofday(&tod, NULL);
	clock_t ticks = tod.tv_sec * CLOCKS_PER_SEC
			+ tod.tv_usec / (1000000 / CLOCKS_PER_SEC);

	if (buffer)
		// There is no operating system, nor child processes, so only
		// the user CPU time is defined.
		*buffer = (struct tms){ ticks, 0, 0, 0 };

	return ticks;
}
