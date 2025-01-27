#include <lely/libc/time.h>
#include <lely/bsp/hw_time.h>
#include <stm32g4xx_hal.h>
#include <main.h>

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	(void)clock_id;
//	struct hw_time t = clock_get_hw_time();
    RTC_TimeTypeDef gTime;
	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	//tp->tv_nsec = t.ticks_ns;
	tp->tv_sec = gTime.Seconds;
	tp->tv_nsec = (gTime.SubSeconds * 1000000000UL) / (gTime.SecondFraction + 1);
return 0;
}

int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
	(void)clock_id;
	RTC_TimeTypeDef sTime;
	sTime.Seconds = tp->tv_sec;
	sTime.SubSeconds = tp->tv_nsec;
	sTime.SecondFraction = 1000 * 1000 * 1000;
return HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}
