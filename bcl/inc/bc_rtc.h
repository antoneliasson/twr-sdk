#ifndef _BC_RTC_H
#define _BC_RTC_H

#include <time.h>
#include <stm32l0xx.h>
#include "bc_common.h"

//! @addtogroup bc_rtc bc_rtc
//! @brief Driver for real-time clock
//! @{

// Note: For performance reasons, the BC_RTC_PREDIV_S value should be a power of
// two.
#define BC_RTC_PREDIV_S 256
#define BC_RTC_PREDIV_A 128

//! @brief Initialize real-time clock

extern int _bc_rtc_writable_semaphore;

void bc_rtc_init(void);

//! @brief RTC date and time structure
typedef struct
{
    uint8_t seconds;     //!< Seconds parameter, from 00 to 59
	uint16_t subseconds; //!< Subsecond downcounter. When it reaches zero, it's reload value is the same as @ref RTC_SYNC_PREDIV
	uint8_t minutes;     //!< Minutes parameter, from 00 to 59
	uint8_t hours;       //!< Hours parameter, 24Hour mode, 00 to 23
	uint8_t week_day;         //!< Day in a week, from 1 to 7
	uint8_t date;        //!< Date in a month, 1 to 31
	uint8_t month;       //!< Month in a year, 1 to 12
	uint16_t year;        //!< Year parameter, 2000 to 2099
	uint32_t timestamp;  //!< Seconds from 01.01.1970 00:00:00
} bc_rtc_t;

/**
 * Obtain current date and time from RTC
 *
 * This function retrieves the current date and time from the RTC peripheral and
 * stores the result broken down in the given struct tm.
 *
 * The function has been optimized for speed. It uses most recent value
 * memoization to amortize run time across successive invocations. Pre-computed
 * tables are used to speed up leap year and year-of-day conversions. Run times
 * measured with 2.1 MHz system clock are as follows:
 *
 *   - 88 us on first use or date register (RTC_DR) change
 *   - 55 us on time register (RTC_TR) change
 *   - 33 us on sub-second register (RTC_SSR) change only
 *
 * Thus, when called repeatedly within a one second interval, the first
 * invocation will complete in 55 us and any subsequent invocations will only
 * take 33 us until the RTC_TR register changes.
 *
 * Warning: The function does not check whether the RTC's shadow registers have
 * been initialized. If invoked in a state where they might not be, e.g., after
 * RTC initialization, system reset, or wake up from deep sleep, you need to
 * perform the check yourself beforehand.
 *
 * Both 12-hour and 24-hour RTC modes are supported.
 *
 * @param[out] tm A pointer to target struct tm variable to hold the result
 */
void bc_rtc_get_datetime(struct tm *tm);

/**
 * Set date and time in RTC
 *
 * This function configures the current date and time in the RTC peripheral.
 * Date and time is passed broken down (struct tm) in the first parameter. A
 * sub-second component is passed as the number of milliseconds in the second
 * parameter.
 *
 * @param[in] tm Struct tm with calendar date and time to set in the RTC
 * @param[in] ms Sub-second time (number of milliseconds, 0 if unknown)
 * @return 0 on success, a negative number on error
 */
int bc_rtc_set_datetime(struct tm *tm, int ms);

//! @brief Convert RTC to timestamp
//! @param[in] rtc Pointer to the RTC date and time structure
//! @return unix timestamp

uint32_t bc_rtc_rtc_to_timestamp(bc_rtc_t *rtc);

//! @brief Enable RTC write protection
//
// This function supports nested invocations. If bc_rtc_enable_write has been
// called repeatedly, calling this function repeatedly will only lock the RTC
// again after all calls to bc_rtc_enable_write have been unrolled.

static inline void bc_rtc_disable_write()
{
	if (--_bc_rtc_writable_semaphore <= 0) {
		_bc_rtc_writable_semaphore = 0;
		RTC->WPR = 0xff;
	}
}

//! @brief Disable RTC write protection

static inline void bc_rtc_enable_write()
{
	++_bc_rtc_writable_semaphore;
	RTC->WPR = 0xca;
	RTC->WPR = 0x53;
}

//! @brief Enable or disable RTC initialization mode
//! @param[in] state Enable when true, disable when false

void bc_rtc_set_init(bool state);

//! @}

#endif // _BC_RTC_H
