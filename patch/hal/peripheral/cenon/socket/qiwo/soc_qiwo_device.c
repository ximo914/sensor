#if defined(__CENON_SOCKET_QIWO_SUPPORT__)
/******************************************************************************
 Copyright (c) 2015 Cenon(Shanghai) Co.,Ltd. 
 All rights reserved.

 [Module Name]:		
 		soc_qiwo_device.c
 [Date]:       
 		2015-08-03
 [Comment]:   	
 		qiwo function relative subroutine.
 [Reversion History]:
 		v1.0
*******************************************************************************/
#define _SOC_QIWO_DEVICE_C_

//custom
#include "soc_qiwo_device.h"

#include "mmi_frm_utility_gprot.h"
#include "app_datetime.h"
#include "dcl_rtc.h"
#include "..\..\gps\cenon_ublox.h"
#include "NwInfoSrv.h"
#include "SmsSrvGprot.h"
#include "TimerEvents.h"
#include "cJSON.h"

/*========================================================================================================
										D E B U G
========================================================================================================*/
#ifndef JOHNSON_SAY
#define JOHNSON_SAY							"[Qian]:"
#endif

#if defined(__CENON_DEBUG_SUPPORT__)
#define DEBUG_UBLOX_FUNC(fmt, arg...)			kal_prompt_trace(MOD_ENG_2, JOHNSON_SAY "----[UBLOX]----[func]----%s\n", __func__)
#define DEBUG_UBLOX_INFO(fmt, arg...)			kal_prompt_trace(MOD_ENG_2, JOHNSON_SAY "----[UBLOX]----[info]----" fmt, ##arg)
#else
#define DEBUG_UBLOX_FUNC() 					//do{}while(0)
#define DEBUG_UBLOX_INFO(fmt, arg...)			//do{}while(0)	
#endif


/*========================================================================================================
										E X T E R N
========================================================================================================*/

extern void mmi_get_gps_num(char*src1, char* src2);
extern void jwcloud_socket_upload_location(MMI_BOOL cell);
extern void jwcloud_socket_upload_pedometer();
extern void goto_yx_calibration_delay(void);
extern void mmi_frm_scrn_close_active_id (void);
extern MMI_BOOL yx_get_device_if_is_on(void);


extern S32 g_cenon_srv_nw_info_signal_strength[MMI_SIM_TOTAL];

/*========================================================================================================
										D E F I N E
========================================================================================================*/

#define J_GPS_UPL_INTERVAL  (5*1000)
#define J_GPS_LOCATION_TRY_TIME  (1*60*1000)
#define J_GPS_LOCATION_TRY_FIRST_TIME  (2*60*1000)
#define J_CELL_UPL_INTERVAL  (60*1000)
#define J_CELL_LOCATION_TRY_TIME  (20*1000)
#define J_GPS_PEDOMETER_INTERVAL  (5*60*1000)
#define J_GPS_UPDATE_PERIOD  (10*60*1000)
#define JWSDK_IMSI_STR_LEN  (17)
#define TIME_UNIT_MIN (60*1000)

#define  jmin(a,b)    ((a>b)?b:a)

/*========================================================================================================
										T Y P E D E F
========================================================================================================*/
#define YX_REPLAY_JSON_KEY_DATA        "data"
#define YX_REPLAY_JSON_KEY_MSG  "msg"
#define YX_REPLAY_JSON_KEY_ERROR_CODE            "code"
#define YX_REPLAY_JSON_KEY_TIME            "time"
#define YX_REPLAY_JSON_KEY_INTERVAL            "interval"


/*========================================================================================================
										V A R I A B L E S
========================================================================================================*/

static U32 g_device_alm_status  = 0;//J_ALM_NULL;
static J_CELL_DATA_ARRAY  g_cell_data_array = {0};
static MMI_BOOL g_gps_located = MMI_FALSE;
static MMI_BOOL g_power_outside = MMI_TRUE;
static J_GPS_DATA  g_gps_data = {0};
static U8 g_gps_satellite = 0; 
static U8  g_speed_v = 0;
static U32 g_location_time = 0;
static MMI_BOOL g_gps_if_located = MMI_FALSE;
static U32 g_cell_time = 0;
static MMI_BOOL g_cell_if_located = MMI_FALSE;
static MMI_BOOL g_gps_if_located_one_time = MMI_FALSE;
#ifdef __CENON_JWSDK_SOCKET_HEART__
static S16 g_heart_rate = 0;
#endif
#ifdef __CENON_JWSDK_SOCKET_PEDOMETER__
static MMI_BOOL g_pedometer_if_update = MMI_FALSE;
#endif
#ifdef __CENON_JWSDK_SOCKET_SLEEP__
J_SLEEP_DATA g_sleep_data = {0};
#endif
MMI_BOOL g_factory_mode = MMI_FALSE;

#define JWSDK_ADM_MEM_SIZE  (9*1024)
static U8 g_adm_mem[JWSDK_ADM_MEM_SIZE+1] = {0};
static KAL_ADM_ID g_adm_id = 0;
static U8 g_upload_type = 0;
static MMI_BOOL g_if_update_time = MMI_TRUE;
YX_CALIBRATION_STATUS yx_calibration_status;
int is_gps_upload = 0;
int is_gps_searching = 0;
static int  gps_upload_interval = 30;
int  g_check_first = 0;
int g_gps_location_success = 0;
/*========================================================================================================
										F U N C T I O N	 
========================================================================================================*/
int yx_get_chick_complete(void)
{
     return g_check_first;
}
void yx_set_chick_complete(int check)
{
    g_check_first = check;
}

void yx_calibration_time_out(void)
{
    is_gps_upload = 0;
    yx_set_calibration_status(YX_CALIBRATION_STATUS_NO_REPLAY);
}
void yx_location_time_out(void)
{

    is_gps_upload = 0;
    
    if(g_gps_location_success)
        yx_set_calibration_status(YX_CALIBRATION_STATUS_LOCATION_SUCCESS_UPLOAD_FAIL);
    else    
        yx_set_calibration_status(YX_CALIBRATION_STATUS_NO_REPLAY);
}

void yx_upload_start_up(void);

YX_CALIBRATION_STATUS yx_get_calibration_status(void)
{
    return yx_calibration_status;
}
void yx_set_calibration_status(YX_CALIBRATION_STATUS status)
{
    yx_calibration_status = status;
}
MMI_BOOL get_yx_if_need_update_time(void){
   return g_if_update_time;
}
void set_yx_if_need_update_time(MMI_BOOL need){
   g_if_update_time = need;
}

U8 get_yx_upload_type(void){
   return g_upload_type;
}
void set_yx_upload_type(U8 type){
   g_upload_type = type;
}

U32 jascii_v(U8* buf, U8 len)
{
    U32 v = 0;
    U8 i =0;

    if(buf == NULL || len == 0){
        kal_prompt_trace(MOD_ENG_2,"jascii_v in is error");
        return v;
    }
    for( i=0; i<len; i++)
    {
        if(buf[i]<'0' || buf[i] >'9')
        {
            continue;
        }
        if (v > 429496729 || ((v == 429496729) && buf[i]>'5' ))
        {
            continue;
        }          
        v = (10*v) + buf[i] - '0';
    }     

    return v;
}

MMI_BOOL jwsdk_device_read_imsi(char* out, U32 len)
{ 
     MMI_BOOL ret = MMI_FALSE;
     if(out && JWSDK_IMSI_STR_LEN <=len)
     {
        ret = MMI_TRUE;
        srv_sim_ctrl_get_imsi(MMI_SIM1, out, len);
     }
     return ret;
}

MMI_BOOL  jwsdk_device_read_imei(char* out, U32 len)
{   
     MMI_BOOL ret = MMI_FALSE;
     if(out && len)
     {        
       //memcpy(out, "123456789012345", 15);
       if (out && 16<=len)
       {
          ret = MMI_TRUE;
          srv_imei_get_imei(MMI_SIM1, out, len);
       }
     }
     kal_prompt_trace(MOD_ENG_2,"imei:%s", (out)?out:"NULL");
     return ret;
}
void cenon_func_datetime_set_timezone(FLOAT curItem);
void jwsdk_device_set_timezone(float timezone)
{
    cenon_func_datetime_set_timezone(timezone);
}

float jwsdk_device_get_timezone()
{
    float time_zone = 0;
    
    time_zone = GetTimeZone(PhnsetGetHomeCity());

    return time_zone;
}

U64 jwsdk_device_get_timestamp()
{
    applib_time_struct time={0};
    applib_time_struct utc_time={0};
    U64 time_stamp = 0;
    U64 time_stamp_utc = 0;
    kal_prompt_trace(MOD_ENG_2,"{jwsdk_device_get_timestamp IN}");
    applib_dt_get_rtc_time(&time);
    applib_dt_rtc_to_utc_with_default_tz(&time, &utc_time);
    
    time_stamp = applib_dt_mytime_2_utc_sec_ext(&time, 0);
    time_stamp_utc =  applib_dt_mytime_2_utc_sec_ext(&utc_time, 0);

    kal_prompt_trace(MOD_ENG_2,"{nYear:%d, nMonth:%d, nDay:%d, nHour:%d, nMin:%d, nSec:%d}", 
                            time.nYear, time.nMonth, time.nDay, time.nHour, time.nMin, time.nSec);
    kal_prompt_trace(MOD_ENG_2,"{time_stamp:%lld}",time_stamp);


    kal_prompt_trace(MOD_ENG_2,"{nYear:%d, nMonth:%d, nDay:%d, nHour:%d, nMin:%d, nSec:%d}", 
                            utc_time.nYear, utc_time.nMonth, utc_time.nDay, utc_time.nHour, utc_time.nMin, utc_time.nSec);
    kal_prompt_trace(MOD_ENG_2,"{time_stamp:%lld}",time_stamp_utc);
    kal_prompt_trace(MOD_ENG_2,"{jwsdk_device_get_timestamp OUT}");
    return time_stamp_utc;    
}

MMI_BOOL jwsdk_device_sync_time(U16 y, U8 mon, U8 d, U8 h, U8 min, U8 s)
{
    RTC_CTRL_SET_TIME_T t = {0};
    
    kal_prompt_trace(MOD_ENG_2,"{y:%d, mon:%d, d:%d, h:%d, m:%d, s:%d}", y, mon, d, h, min, s);

    t.u1Year  = y-2000;
    t.u1Mon = mon;
    t.u1Day =d;
    t.u1Hour = h;
    t.u1Min = min;
    t.u1Sec = s;
    uem_rtc_settime(&t);

    return MMI_TRUE;
}

MMI_BOOL jwsdk_device_sync_timestamp(U64 time_stamp)
{
    applib_time_struct time = {0};
    applib_time_struct rtc_time = {0};
    applib_dt_utc_sec_2_mytime_ext(time_stamp, &time, 0);
    applib_dt_utc_to_rtc_with_default_tz(&time, &rtc_time);
    kal_prompt_trace(MOD_ENG_2,"time_stamp:%lld,y:%d, mon:%d, d:%d,h:%d,min:%d,s:%d",
                             time_stamp,
                             time.nYear,
                             time.nMonth,
                             time.nDay,
                             time.nHour,
                             time.nMin,
                             time.nSec);
    kal_prompt_trace(MOD_ENG_2,"time_stamp:%lld,y:%d, mon:%d, d:%d,h:%d,min:%d,s:%d",
                             time_stamp,
                             rtc_time.nYear,
                             rtc_time.nMonth,
                             rtc_time.nDay,
                             rtc_time.nHour,
                             rtc_time.nMin,
                             rtc_time.nSec);
    jwsdk_device_sync_time( rtc_time.nYear,
                             rtc_time.nMonth,
                             rtc_time.nDay,
                             rtc_time.nHour,
                             rtc_time.nMin,
                             rtc_time.nSec);
}

 MMI_BOOL jwsdk_device_power_enable(MMI_BOOL enable)
{
    kal_prompt_trace(MOD_ENG_2,"{enable:%d}\n", enable);
    return MMI_TRUE;
}

U8  jwsdk_device_speed_v()
{
     kal_prompt_trace(MOD_ENG_2,"{g_speed_v:%d}\n", g_speed_v);
    return g_speed_v;
}

MMI_BOOL jwsdk_device_gps_is_on()
{
    MMI_BOOL is_on = MDrv_Ublox_Get_Power_OnOff_Proxy();
    kal_prompt_trace(MOD_ENG_2,"{is_on:%d}\n", is_on);
    return is_on;
}

MMI_BOOL jwsdk_device_is_factory_mode()
{
    kal_prompt_trace(MOD_ENG_2,"{g_factory_mode:%d}", g_factory_mode);
    return g_factory_mode;
}


MMI_BOOL jwsdk_device_gps_set_onoff(MMI_BOOL  enable)
{
    MMI_BOOL on_off = MDrv_Ublox_Get_Power_OnOff_Proxy();

    kal_prompt_trace(MOD_ENG_2,"{on_off:%d, enable:%d}\n", on_off, enable);

    if(on_off != enable)
    {
        MDrv_Ublox_Set_Power_OnOff_Proxy(enable);
    }
    return MDrv_Ublox_Get_Power_OnOff_Proxy();
}


static void  jwsdk_device_gps_set_on()
{
      jwsdk_device_gps_set_onoff(MMI_TRUE);
}

MMI_BOOL  jwsdk_device_gps_update_data()
{
     MMI_BOOL ret = MMI_FALSE;
     
     ST_GPS_RMC_DATA GPS_RMC_DATA={0};
     ST_GPS_RMC_STRING GPS_RMC_STRING={0};
     J_GPS_DATA* gps_data =NULL;
     MMI_BOOL valid = MMI_FALSE;

    MDrv_Ublox_NMEA_Read_UartData();
    
    valid = MDrv_Ublox_NMEA_Get_RMC_Data(g_uart_ubolx_RMC, &GPS_RMC_DATA, &GPS_RMC_STRING);

    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:valid=%d \n",valid);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:date=%d-%d-%d \n", GPS_RMC_DATA.D.year, GPS_RMC_DATA.D.month, GPS_RMC_DATA.D.day);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:time=%d:%d:%d \n", GPS_RMC_DATA.D.hour, GPS_RMC_DATA.D.minute, GPS_RMC_DATA.D.second);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:str_latitude=%s,str_longitude=%s, \n", GPS_RMC_STRING.str_latitude, GPS_RMC_STRING.str_longitude);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:lat_Degree=%d,lat_Cent=%d,lat_Second=%d  \n",  GPS_RMC_DATA.latitude_Degree, GPS_RMC_DATA.latitude_Cent, GPS_RMC_DATA.latitude_Second);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:lon_Degree=%d,lon_Cent=%d,lon_Second=%d  \n",  GPS_RMC_DATA.longitude_Degree, GPS_RMC_DATA.longitude_Cent, GPS_RMC_DATA.longitude_Second);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:speed=%d \n", GPS_RMC_DATA.speed);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:direction=%d \n", GPS_RMC_DATA.direction);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:height=%d \n", GPS_RMC_DATA.height);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:NS=%d \n", GPS_RMC_DATA.NS);
    kal_prompt_trace(MOD_ENG_2,"[GPS Raw Data]:EW=%d \n",GPS_RMC_DATA.EW);
    
     if (valid)
     {
        U8 i = 0;
        U8 j = 0;
        
        memset(&g_gps_data,0x00,sizeof(J_GPS_DATA));
        
        g_gps_data.hour = GPS_RMC_DATA.D.hour;
        g_gps_data.min = GPS_RMC_DATA.D.minute;
        g_gps_data.sec = GPS_RMC_DATA.D.second;
        g_gps_data.time_stamp_v = jwsdk_device_get_timestamp();
        for(i = 0;( i< RMC_LEN_LONGTITUDE && j< JWSDK_GPS_LONGITUDE_LEN); i++)
        {
            if (('0' <= GPS_RMC_STRING.str_longitude[i] && '9' >=  GPS_RMC_STRING.str_longitude[i])||'.' ==  GPS_RMC_STRING.str_longitude[i])
            {
                 g_gps_data.longitude[j] = GPS_RMC_STRING.str_longitude[i];
                 j++;
            }          
        }

        j = 0;
        for(i = 0; (i< RMC_LEN_LATITUDE && j < JWSDK_GPS_LATITUDE_LEN); i++)
        {
            if (('0' <= GPS_RMC_STRING.str_latitude[i] && '9' >=  GPS_RMC_STRING.str_latitude[i])  || '.' ==  GPS_RMC_STRING.str_latitude[i])
            {
                 g_gps_data.latitude[j] = GPS_RMC_STRING.str_latitude[i];
                 j++;
            }          
        }
       
        g_gps_data.flag = (0x30)|(0x06)|((valid)?(0x00):(0x01));
        g_gps_data.v = jascii_v(GPS_RMC_STRING.str_speed,strlen(GPS_RMC_STRING.str_speed));//(U8)GPS_RMC_DATA.speed;
        g_gps_data.direction = jascii_v(GPS_RMC_STRING.str_direction,strlen(GPS_RMC_STRING.str_direction));//(U8)GPS_RMC_DATA.direction;
        g_gps_data.day = GPS_RMC_DATA.D.day;
        g_gps_data.mon =GPS_RMC_DATA.D.month;
        
        if(GPS_RMC_DATA.D.year)
        {
           g_gps_data.year = GPS_RMC_DATA.D.year-2000;
        }
        
        g_gps_data.location = valid;

        ret = MMI_TRUE;

        kal_prompt_trace(MOD_ENG_2,"{year:%d, mon:%d, day:%d, hour:%d, min:%d, sec:%d, v:%d, direction:%d}\n",\
                                   g_gps_data.year,\
                                   g_gps_data.mon,\
                                   g_gps_data.day, \
                                   g_gps_data.hour, \
                                   g_gps_data.min, \
                                   g_gps_data.sec,\
                                   g_gps_data.v,\
                                   g_gps_data.direction);

         kal_prompt_trace(MOD_ENG_2,"{location:%d, flag:%d, longitude:%s, latitude:%s} \n", \
                                  g_gps_data.location, \
                                  g_gps_data.flag, \
                                  g_gps_data.longitude, \
                                  g_gps_data.latitude);      
    
    }   

     return ret;
}

MMI_BOOL jwsdk_device_read_gps_data(J_GPS_DATA* gps)
{
    //31.577887,120.289055
    MMI_BOOL  ret = MMI_FALSE;
    kal_prompt_trace(MOD_ENG_2,"{jwsdk_device_read_gps_data IN}\n");
    if(gps != NULL)
    {
        kal_prompt_trace(MOD_ENG_2,"{jwsdk_device_read_gps_data gps != NULL}\n");
        memcpy(gps, &g_gps_data, sizeof(J_GPS_DATA));
        ret = MMI_TRUE;
    }
    kal_prompt_trace(MOD_ENG_2,"{jwsdk_device_read_gps_data result:%d}\n", ret);
    return ret ;

}

U8 jwsdk_device_read_gps_signal()
{
   U8 value = 0;
   jwsdk_device_gps_update_data();
   value = g_gps_satellite;
   kal_prompt_trace(MOD_ENG_2,"{value:%d}\n", value);
   return jmin(value, 99);
}


U32 jwsdk_device_read_alm_status(U32 mask)
{
    kal_prompt_trace(MOD_ENG_2,"{g_device_alm_status:%x, mask:%x}", g_device_alm_status, mask);
    return (g_device_alm_status & mask);
}


#define Q_DBM(dBm) ((U32)(dBm) * 4)

static  U8  jwsdk_get_percentage_from_gsm_rssi(U32 rssi_in_qdBm)
{

    U32 percentage = 0;
    

    if (rssi_in_qdBm < Q_DBM(-107))
    {
        percentage = 0;
    }
    else if (rssi_in_qdBm < Q_DBM(-77))
    {
        percentage =
            ((rssi_in_qdBm - Q_DBM(-107)) * 100) / (Q_DBM(-77) - Q_DBM(-107));
    }
    else
    {
        percentage = 100;
    }

    return (U8)percentage;
}

U8 jwsdk_device_read_gsm_signal()

{
    U8 value = 0;
    srv_nw_info_cntx_struct *cntx;
    cntx = srv_nw_info_get_cntx(MMI_SIM1);
    kal_prompt_trace(MOD_ENG_2,"{cntx:%x}", cntx);
    if(cntx)
    {       
        value =  jwsdk_get_percentage_from_gsm_rssi(cntx->signal.gsm_RSSI_in_qdBm);
        kal_prompt_trace(MOD_ENG_2,"{rssi:%c%d, percentage:%d%}", \
                                 (cntx->signal.gsm_RSSI_in_qdBm>=0)?('+'):('-'), \
                                 (cntx->signal.gsm_RSSI_in_qdBm>=0)?cntx->signal.gsm_RSSI_in_qdBm:(-cntx->signal.gsm_RSSI_in_qdBm),
                                 value);
    }

   kal_prompt_trace(MOD_ENG_2,"{g_cenon_srv_nw_info_signal_strength:%d, }\n", g_cenon_srv_nw_info_signal_strength[MMI_SIM1]);

   return jmin(value, 99);
}

MMI_BOOL jwsdk_device_update_cell_data(MMI_BOOL enable)
{   
    if(enable)
    {
        MDrv_Soc_Device_Cell_Request();
    }
    else 
    {     
        MDrv_Soc_Device_Cell_Cancel();
    }

    kal_prompt_trace(MOD_ENG_2,"{enable:%d}\n", enable);

    return MMI_TRUE;
}

MMI_BOOL jwsdk_device_write_cell_data(J_CELL_DATA_ARRAY* in)
{
    
    if(in)
    {
        kal_prompt_trace(MOD_ENG_2,"cell count:%d", in->count);
        memcpy(&g_cell_data_array, in, sizeof(J_CELL_DATA_ARRAY));
        g_cell_data_array.time_stamp_v = jwsdk_device_get_timestamp();
    }
    return MMI_TRUE;
}

MMI_BOOL jwsdk_device_read_cell_data(J_CELL_DATA_ARRAY* out)
{
     U8 i =0;
     
     for(i = 0; (i < g_cell_data_array.count); i++)
     {

         kal_prompt_trace(MOD_ENG_2,"{index:%d, mcc:%x, mnc:%x,lac:%x, ci:%x,rssi:%x}",
                     i,
                     g_cell_data_array.data_array[i].mcc,
                     g_cell_data_array.data_array[i].mnc,
                     g_cell_data_array.data_array[i].lac,
                     g_cell_data_array.data_array[i].ci,
                     g_cell_data_array.data_array[i].rssi);              
     }
     if (out) 
     {
        memcpy(out,  &g_cell_data_array, sizeof(J_CELL_DATA_ARRAY));
     }
     return MMI_TRUE;
}

static void jwsdk_device_send_sms_success_callback(srv_sms_callback_struct* callback_data)
{  
   kal_prompt_trace(MOD_ENG_2,"{Send Sms Result:%d, cause:%d}", callback_data->result, callback_data->cause);
}


MMI_BOOL jwsdk_device_reset()
{
    kal_prompt_trace(MOD_ENG_2,"assert~~~~~");
    //assert(0);
    EXT_ASSERT(0,0,0,0);
    return MMI_TRUE;
}
void jwsdk_device_set_factory_mode(MMI_BOOL mode){
    g_factory_mode = mode;
}

MMI_BOOL jwsdk_device_get_gps_double(double * fLon, double * fLat)
{ 
    MMI_BOOL  ret = MMI_FALSE;
    J_GPS_DATA gps_data;
    int ilon,ilat;
    
    jwsdk_device_read_gps_data(&gps_data);
    
    ilon = jascii_v(gps_data.longitude, strlen(gps_data.longitude) + 1);
	ilat = jascii_v(gps_data.latitude, strlen(gps_data.latitude) + 1);
    
    if(0 < ilon && 0 < ilat )
    {
		*fLon = (ilon/1000000)+((((double)(ilon%1000000))/10000)/60);
		*fLat = (ilat/1000000)+((((double)(ilat%1000000))/10000)/60);
        kal_prompt_trace(MOD_ENG_2,"{ilon:%d, ilat:%d, fLon:%6f, fLat:%6f}", ilon, ilat, *fLon, *fLat);
        ret = MMI_TRUE;
    } 
    return ret;
}

MMI_BOOL jwsdk_device_get_gps_double_factory_mode(double * fLon, double * fLat)
{ 
    MMI_BOOL  ret = MMI_FALSE;
    if(jwsdk_device_gps_update_data())
    {
        ret = jwsdk_device_get_gps_double(fLon, fLat);
    }
    return ret;
}

void jwsdk_update_cell_data()
{
    if(!MDrv_Soc_Device_Sim_Get_Valid())
	{
        return;
    }
    jwsdk_device_update_cell_data(MMI_TRUE);
    StartTimer(TIMER_ID_JWSDK_GET_CELL, J_CELL_UPL_INTERVAL, jwsdk_update_cell_data);
}

void jwsdk_get_gps_data(void)
{
    kal_prompt_trace(MOD_ENG_2,"{g_location_time:%d}", g_location_time/J_GPS_UPL_INTERVAL);
    do{
        if(MMI_TRUE == jwsdk_device_gps_update_data())
        {
            kal_prompt_trace(MOD_ENG_2,"{jwsdk_get_gps_data get gps success}");
            g_gps_if_located_one_time = MMI_TRUE;
            g_gps_location_success = 1;
            break;
        }
        g_location_time += J_GPS_UPL_INTERVAL;
        if(MMI_TRUE == g_gps_if_located_one_time){
            if(J_GPS_LOCATION_TRY_TIME <= g_location_time){
                break;
            }
        }else{
            if(J_GPS_LOCATION_TRY_FIRST_TIME <= g_location_time){
                break;
            }
        }
        StopTimer(TIMER_ID_JWSDK_GET_GPS);
        StartTimer(TIMER_ID_JWSDK_GET_GPS, J_GPS_UPL_INTERVAL, jwsdk_get_gps_data);
        return;
    }while(0);
    if(!g_factory_mode){
        kal_prompt_trace(MOD_ENG_2,"{jwsdk_get_gps_data set gps OFF}");
        jwsdk_device_gps_set_onoff(MMI_FALSE);
    }
    MDrv_Soc_Gps_Socket_Entry_Proxy();
}
void jwsdk_start_get_gps(void)
{
    g_location_time = 0;
    is_gps_searching = 1;
    g_gps_location_success = 0;
    if(!MDrv_Soc_Device_Sim_Get_Valid())
	{
	    yx_set_calibration_status(YX_CALIBRATION_STATUS_NO_SIM_CARD);
        return;
    }
    if(!jwsdk_device_gps_is_on()){
        jwsdk_device_gps_set_onoff(MMI_TRUE);
    }
    jwsdk_get_gps_data();
}
void jwsdk_upload_gps_period(void)
{
    if(is_gps_searching == 1 || is_gps_upload == 1){
        kal_prompt_trace(MOD_ENG_2,"{jwsdk_upload_gps_period GPS is Running do nothing}");
    }else{
        set_yx_upload_type(0);
        jwsdk_start_get_gps();
    }
    StopTimer(TIMER_ID_JWSDK_UPDATE_GPS_PEROID);
    StartTimer(TIMER_ID_JWSDK_UPDATE_GPS_PEROID, gps_upload_interval*TIME_UNIT_MIN, jwsdk_upload_gps_period);
}

void jwsdk_upload_cell_only_period(void)
{
    set_yx_upload_type(0);
    MDrv_Soc_Gps_Socket_Entry_Proxy();
    StopTimer(TIMER_ID_JWSDK_UPDATE_CELL_ONLY_PEROID);
    StartTimer(TIMER_ID_JWSDK_UPDATE_CELL_ONLY_PEROID, gps_upload_interval*TIME_UNIT_MIN, jwsdk_upload_cell_only_period);
}

void jwsdk_upload_gps_for_sos(void)
{
    set_yx_upload_type(1);
    if(g_gps_if_located_one_time){
        StopTimer(TIMER_ID_CLOCK_CALIBRATION_TIME_OUT);
        StartTimer(TIMER_ID_CLOCK_CALIBRATION_TIME_OUT, 2*60*1000, yx_location_time_out);
    }else{
        StopTimer(TIMER_ID_CLOCK_CALIBRATION_TIME_OUT);
        StartTimer(TIMER_ID_CLOCK_CALIBRATION_TIME_OUT, 3*60*1000, yx_location_time_out);
    }
    jwsdk_start_get_gps();
}

void yx_set_time(U8* pdu)
{
    cJSON*  json_data = NULL;
    cJSON* code = NULL;
    cJSON* time = NULL;
    cJSON* msg = NULL;
    char* parse_end = NULL;
    char* json_data_str = NULL;
    MMI_BOOL set_result = MMI_FALSE;
    
    kal_prompt_trace(MOD_ENG_2,"{yx_set_time:IN}");
    set_yx_if_need_update_time(MMI_FALSE);
    json_data_str = (char*)strstr(pdu, "{");

    if (json_data_str)
    {  
        kal_prompt_trace(MOD_ENG_2,"{json_data_str:%s}",json_data_str);
        json_data = cJSON_ParseWithOpts(json_data_str, &parse_end, 0);
    }
    else
    {
        yx_set_calibration_status(YX_CALIBRATION_STATUS_REPLAY_ERROR);
        kal_prompt_trace(MOD_ENG_2,"{json_data_str == NULL}");
    }
   

    kal_prompt_trace(MOD_ENG_2,"{json_data:%x}", json_data);

    if(json_data)
    {
        code = cJSON_GetObjectItem(json_data, YX_REPLAY_JSON_KEY_ERROR_CODE);
        time = cJSON_GetObjectItem(json_data, YX_REPLAY_JSON_KEY_TIME);
        msg = cJSON_GetObjectItem(json_data, YX_REPLAY_JSON_KEY_MSG);
        if(code)
        {
            kal_prompt_trace(MOD_ENG_2,"{code != null ||code->valueint = %d}",code->valueint);
        }
        else
        {
            kal_prompt_trace(MOD_ENG_2,"{code == null}");
        }
        if(msg){
            kal_prompt_trace(MOD_ENG_2,"{msg != null msg->valuestring = %s}",msg->valuestring);
        }
        else
        {
            kal_prompt_trace(MOD_ENG_2,"{msg == null}");
        }
        if (time && cJSON_Number == time->type)
        {
            yx_set_calibration_status(YX_CALIBRATION_STATUS_SUCCESS);
            kal_prompt_trace(MOD_ENG_2,"{set time,time->valuelong = %lld}",time->valuelong);
            jwsdk_device_sync_timestamp((U64)(time->valuelong));
            set_result = MMI_TRUE;
            
        }
        else
        {
            yx_set_calibration_status(YX_CALIBRATION_STATUS_REPLAY_ERROR);
            kal_prompt_trace(MOD_ENG_2,"{time == null || cJSON_Number != time->type}");
        }
    }
    else
    {
        yx_set_calibration_status(YX_CALIBRATION_STATUS_REPLAY_ERROR);
        kal_prompt_trace(MOD_ENG_2,"{json_data == null}");
    }
    g_check_first = 1;
    cJSON_Delete(json_data);
    kal_prompt_trace(MOD_ENG_2,"{yx_set_time:out}"); 
}

void yx_set_upload_interval(U8* pdu)
{
    cJSON*  json_data = NULL;
    cJSON* data = NULL;
    char* parse_end = NULL;
    char* json_data_str = NULL;
    MMI_BOOL set_result = MMI_FALSE;
    
    kal_prompt_trace(MOD_ENG_2,"{yx_set_upload_interval:IN}");
    set_yx_if_need_update_time(MMI_FALSE);
    json_data_str = (char*)strstr(pdu, "{");

    if (json_data_str)
    {  
        kal_prompt_trace(MOD_ENG_2,"{json_data_str:%s}",json_data_str);
        json_data = cJSON_ParseWithOpts(json_data_str, &parse_end, 0);
    }
    else
    {
        if(g_gps_location_success)
            yx_set_calibration_status(YX_CALIBRATION_STATUS_LOCATION_SUCCESS_UPLOAD_FAIL);
        else    
            yx_set_calibration_status(YX_CALIBRATION_STATUS_REPLAY_ERROR);
        kal_prompt_trace(MOD_ENG_2,"{json_data_str == NULL}");
    }
   

    kal_prompt_trace(MOD_ENG_2,"{json_data:%x}", json_data);

    if(json_data)
    {
        data = cJSON_GetObjectItem(json_data, YX_REPLAY_JSON_KEY_DATA);
        if(data)
        {
            cJSON* interval = cJSON_GetObjectItem(data, YX_REPLAY_JSON_KEY_INTERVAL);
            
            if(interval && interval->valueint > 0){
                kal_prompt_trace(MOD_ENG_2,"{gps_upload_interval = %d , interval->valueint =%d}", gps_upload_interval, interval->valueint);
                yx_set_calibration_status(YX_LOCATION_STATUS_SUCCESS);
                if(interval->valueint != gps_upload_interval){
                    kal_prompt_trace(MOD_ENG_2,"{reset gps_upload_interval}");
                    if(interval->valueint < 5)
                        gps_upload_interval = 5;
                    else
                        gps_upload_interval = interval->valueint;
                }
            }else{
                kal_prompt_trace(MOD_ENG_2,"{interval == null}");
            } 
            
        }
        else
        {
            kal_prompt_trace(MOD_ENG_2,"{data == null}");
        }
       
    }
    else
    {
        if(g_gps_location_success)
            yx_set_calibration_status(YX_CALIBRATION_STATUS_LOCATION_SUCCESS_UPLOAD_FAIL);
        else    
            yx_set_calibration_status(YX_CALIBRATION_STATUS_REPLAY_ERROR);
        kal_prompt_trace(MOD_ENG_2,"{json_data == null}");
    }
 
    cJSON_Delete(json_data);
    kal_prompt_trace(MOD_ENG_2,"{yx_set_upload_interval:out}"); 
}

static void* jwsdk_adm_malloc(size_t sz)
{
    if (0 != g_adm_id)
    {
       return kal_adm_alloc(g_adm_id, sz);
    }
    return NULL;
}

static  void jwsdk_adm_free(void *ptr)
{
     if (0 != g_adm_id)
     {
         kal_adm_free(g_adm_id,  ptr);
      }
}

      
static void jwsdk_adm_mem_init()
{
     
     g_adm_id = kal_adm_create( g_adm_mem,
                                                      JWSDK_ADM_MEM_SIZE,
                                                      NULL,
                                                      KAL_TRUE);
     kal_prompt_trace(MOD_ENG_2,"{g_adm_id:%d}", g_adm_id);
     
}

void yx_power_on_calibration(void)
{
	#ifdef __XI_SUPPORT__
        return;
    #endif
    if(yx_get_device_if_is_on())
    {
        cJSON_Hooks json_hook = {0};
        jwsdk_device_set_timezone(8);
        jwsdk_adm_mem_init();
        json_hook.free_fn = jwsdk_adm_free;
        json_hook.malloc_fn = jwsdk_adm_malloc;       
        cJSON_InitHooks(&json_hook);
        yx_set_calibration_status(YX_CALIBRATION_STATUS_RUNNING);
        StartTimer(TIMER_ID_CLOCK_CALIBRATION_TIME_OUT, 3*60*1000, yx_calibration_time_out);
        goto_yx_calibration_delay();
        MDrv_Soc_Gps_Socket_Entry_Proxy();
    }
}
void yx_upload_start_up(void)
{          
    jwsdk_update_cell_data();
    jwsdk_upload_gps_period();
}
/*========================================================================================================
										The End 
========================================================================================================*/
#endif//#if defined(__CENON_SOCKET_QIWO_SUPPORT__)

