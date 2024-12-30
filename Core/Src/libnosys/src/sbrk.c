#include <errno.h>

extern char _pvHeapStart[];
extern char _pvHeapLimit[];

char *__end_of_heap = _pvHeapStart;

void *
_sbrk(int incr)
{
	// Return an error if the new break falls outside the heap.
	if (__end_of_heap + incr < _pvHeapStart
			|| __end_of_heap + incr > _pvHeapLimit) {
		errno = ENOMEM;
		return (void *)-1;
	}

	void *ptr = __end_of_heap;
	__end_of_heap += incr;
	return ptr;
}
