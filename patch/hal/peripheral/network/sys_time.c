#include "port.h"
#include "sys_time.h"


static void update_sys_time_callback(void *msg)
{
    time_data_t utc, rtc;
    time_t ts ;

    memcpy(&ts, ((uint8_t *)msg + sizeof(local_para_struct)), sizeof(time_t));
    xi_trace("ts:%d", ts);
    utc_second_to_systime(ts, &utc);
    utc_to_rtc_zone(&utc, &rtc);
    xi_trace("year:%d,month:%d,day:%d,hour:%d,min:%d,second:%d", rtc.nYear,
               rtc.nMonth, rtc.nDay, rtc.nHour, rtc.nMin, rtc.nSec);
    mmi_dt_set_dt((const MYTIME *)&rtc, NULL, NULL);
}

static void sys_time_init()
{
   // mmi_frm_set_protocol_event_handler(MSG_ID_TIME_UPDATE, (PsIntFuncPtr)update_sys_time_callback, false);
}

time_t get_sys_timestamp()
{
    time_data_t rtc;
    time_data_t utc;
    tm_t tm;
    time_t ret;

    get_sys_rtc_time(&rtc);
    rtc_to_utc_zone(&rtc, &utc);

    ret = utc_to_timestamp(&utc);
    return ret;
}

void update_sys_time(time_t ts)
{
    #if 0
    sys_time_init();
    port_task_send_msg(MOD_MMI, MSG_ID_TIME_UPDATE, &ts, sizeof(time_t));
    #endif
}

bool cli_get_sys_timestamp(char *data, uint32_t size)
{
    uint32_t timestamp;

    timestamp = get_sys_timestamp();
    xi_trace("time:%d", timestamp);

    return false;
}

bool cli_set_sys_timestamp(char *data, uint32_t size)
{
    uint32_t ts;

    update_sys_time(1536839922);
    ts = get_sys_timestamp();
    xi_trace("new_time:%d", ts);
    return false;
}



