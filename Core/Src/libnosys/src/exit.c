//#include <chip.h>

void
_exit(int status)
{
	(void)status;

	NVIC_SystemReset();
}
