#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include "nosys.h"
#include "main.h"
ssize_t
_write(int fildes, const void *buf, size_t nbyte)
{
HAL_UART_Transmit(&huart2, buf, nbyte, 10);
//printf()
	if (fildes != STDOUT_FILENO && fildes != STDERR_FILENO) {
		errno = EBADF;
		return -1;
	}

	const char *begin = buf;
	const char *end = begin + nbyte;

	const char *bp = begin;
	while (bp < end) {
/*
		int ret = putchar(*bp);
//		size_t n = Chip_UART_SendRB(LPC_UART0, &__libnosys_txring, bp,end - bp);
		if (ret == EOF)
		{
			break;
		}
*/
		bp += 1;
	}

	if (nbyte && bp == begin) {
		errno = EAGAIN;
		return -1;
	}
	return bp - begin;
}
