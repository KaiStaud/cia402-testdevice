#include <errno.h>
#include <unistd.h>

#include "nosys.h"

ssize_t
_read(int fildes, void *buf, size_t nbyte)
{
	/*
	if (fildes != STDIN_FILENO) {
		errno = EBADF;
		return -1;
	}

	char *begin = buf;
	char *end = begin + nbyte;

	char *bp = begin;
	while (bp < end) {
		size_t n = Chip_UART_ReadRB(LPC_UART0, &__libnosys_rxring, bp,
				end - bp);
		if (!n)
			break;
		bp += n;
	}

	if (nbyte && bp == begin) {
		errno = EAGAIN;
		return -1;
	}
	return bp - begin;
	*/
	return 0;
}
