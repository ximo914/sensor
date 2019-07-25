#ifndef _LIB_TIME_H_
#define _LIB_TIME_H_

#include "rtc_sw_new.h"
#include "app_datetime.h"

typedef unsigned int time_t;

typedef struct {
    uint16_t    nYear;
    uint8_t     nMonth;
    uint8_t     nDay;
    uint8_t     nHour;
    uint8_t     nMin;
    uint8_t     nSec;
    uint8_t     dayindex; /*0=sunday*/
} time_data_t;

typedef struct  {
    uint32_t    tm_sec;     /* Seconds: 0-59 (K&R says 0-61?) */
    uint32_t    tm_min;     /* Minutes: 0-59 */
    uint32_t    tm_hour;    /* Hours since midnight: 0-23 */
    uint32_t    tm_mday;    /* Day of the month: 1-31 */
    uint32_t    tm_mon;     /* Months *since* january: 0-11 */
    uint32_t    tm_year;    /* Years since 1900 */
    uint32_t    tm_wday;    /* Days since Sunday (0-6) */
    uint32_t    tm_yday;    /* Days since Jan. 1: 0-365 */
    uint32_t    tm_isdst;   /* +1 Daylight Savings Time, 0 No DST,
                            * -1 don't know */
} tm_t;

typedef struct {
    uint8_t     rtc_sec;    /* seconds after the minute   - [0,59]  */
    uint8_t     rtc_min;    /* minutes after the hour     - [0,59]  */
    uint8_t     rtc_hour;   /* hours after the midnight   - [0,23]  */
    uint8_t     rtc_day;    /* day of the month           - [1,31]  */
    uint8_t     rtc_mon;    /* months                      - [1,12] */
    uint8_t     rtc_wday;   /* days in a week             - [1,7] */
    uint8_t     rtc_year;   /* years                      - [0,127] */
} sys_rtc_t;


#define get_sys_rtc_time(rtc)                            applib_dt_get_rtc_time((applib_time_struct*)rtc)
#define rtc_to_utc_zone(rtc,utc)                         applib_dt_rtc_to_utc_with_default_tz((applib_time_struct *)rtc,(applib_time_struct *)utc)
#define utc_to_rtc_zone(utc,rtc)                         applib_dt_utc_to_rtc_with_default_tz((applib_time_struct *)utc,(applib_time_struct *)rtc)
#define utc_to_timestamp(utc_time)                       applib_dt_mytime_2_utc_sec((applib_time_struct*)utc_time,false)
#define utc_second_to_systime(ts,utc)                    applib_dt_utc_sec_2_mytime(ts, (applib_time_struct*)utc, false);



extern void   update_sys_time();



#endif
