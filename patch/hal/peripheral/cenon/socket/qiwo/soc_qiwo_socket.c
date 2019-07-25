#if defined(__CENON_SOCKET_QIWO_SUPPORT__)
/******************************************************************************
 Copyright (c) 2015 Cenon(Shanghai) Co.,Ltd. 
 All rights reserved.

 [Module Name]:		
 		soc_qiwo_socket.c
 [Date]:       
 		18-05-2015
 [Comment]:   	
 		socket relative subroutine.
 [Reversion History]:
 		v1.0
*******************************************************************************/
#define _SOC_QIWO_SOCKET_C_

//#include 	"kal_release.h"
#include "stack_common.h"  
#include "stack_msgs.h"
#include "app_ltlcom.h"       /* Task message communiction */
#include "syscomp_config.h"

#include "drv_comm.h"
#include "keypad_hw.h"
#include "drv_features.h"
#include "keypad.h"
#include "dcl.h"
#include "gpt_sw.h"
#include "intrCtrl.h"
#include "kbd_table.h"   

#include "drv_trc.h"
#include "task_main_func.h"
#include "stack_config.h"
#include "kal_trace.h"
#include "stack_ltlcom.h"

#include "kal_general_types.h"
#include "kal_public_api.h"
#include "kal_public_defs.h"
#include "sw_keypad.h"
#include "stdlib.h"
#include "stdio.h"

#include "MMIDataType.h"
#include "MMITimer.h"
#include <TimerEvents.h>  
#include "GlobalMenuItems.h"

/* for socket */
#include "McfSocket.h"
#include "soc_api.h"
#include "soc_consts.h"
#include "app2soc_struct.h"
#include "soc2tcpip_struct.h"
#include "mnsms_struct.h"
#include "cbm_api.h"
#include "cJSON.h"
//custom
#include "..\hal\peripheral\cenon\socket\soc_platform.h"
#include "soc_qiwo_socket.h"
#include "soc_qiwo_device.h"
#include "..\..\gps\cenon_ublox.h"
/*========================================================================================================
										D E B U G
========================================================================================================*/
#ifndef TKL_SAY
#define TKL_SAY							"[tkl]:"
#endif

#if defined(__CENON_DEBUG_SUPPORT__)
#define DEBUG_SOC_FUNC(fmt, arg...)			kal_prompt_trace(MOD_SOC, TKL_SAY "----[SOC]----[SOC]----[func]----%s\n", __func__)
#define DEBUG_SOC_INFO(fmt, arg...)			kal_prompt_trace(MOD_SOC, TKL_SAY "----[SOC]----[SOC]----[info]----" fmt, ##arg)
#else
#define DEBUG_SOC_FUNC() 					//do{}while(0)
#define DEBUG_SOC_INFO(fmt, arg...)			//do{}while(0)	
#endif
#define DEBUG_SOC_WARN(fmt, arg...)			kal_prompt_trace(MOD_SOC, TKL_SAY "----[SOC]----[SOC]----[!!!!!!!!!!!!!!!!!!!!!!!]--------" fmt, ##arg)
/*========================================================================================================
										E X T E R N
========================================================================================================*/
//extern void mmi_frm_set_protocol_event_handler(U16 eventID, PsIntFuncPtr funcPtr, MMI_BOOL isMultiHandler);
extern MMI_BOOL srv_imei_get_imei(mmi_sim_enum sim, CHAR *out_buffer, U32 buffer_size);
extern void MDrv_Socket_Event_Handler(void *msg_ptr);
extern U16 g_gps_upload_type;
extern char g_str_longitude[20];
extern char g_str_latitude[20];
extern kal_uint8 bt40_mac_addr[6];
extern int is_gps_searching;

/*========================================================================================================
										D E F I N E
========================================================================================================*/
#define SOC_GPS_REENTRY_TIME		(300000)	// ÿ5���ӽ���һ������(������ӷ�����ʧ��)
#define SOC_GPS_RECONNECT_TIME		(300000)	//ÿ 5���Ӽ��һ���Ƿ����ӷ������ɹ�
#define PAYLOAD_MAX_CELL_COUNT      7 
#define PAYLOAD_LEN_5               (5 +1)      
#if 0
#define SOC_GPS_TEST_FOR_SERVER 1
#endif

#define PACKAGE_BUFFER_LEN  		 1024	//  1 KB		ÿ���ְ�����󳤶�
#define PACKAGE_BUFFER_MAX_LEN 		1024*6	// 6 KB	,	���ݰ���󳤶�����֬����34+23*255 = 5899  byte
#define YX_IMEI_STR_LEN  (16)

//===================
// ���ĸ������ֽ���
//===================
#define PACKET_DATA_HEADER_LEN  	3					// unit: byte
#define PACKAGE_LENTH_LEN			2					// unit: byte
#define PACKET_DATA_MSGID_LEN 		1					// unit: byte
#define PACKET_DATA_TAIL_LEN 		3					// unit: byte
#ifdef __CENON_YX_FOREIGN_VERSION__
#define YX_HTTP_HEADER_CUSTOM  "POST %s HTTP/1.1\r\n\
Content-Length: %d\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Host: 47.88.6.149:30458\r\n\
User-Agent: Fiddler\r\n\
Connection: keep-alive\r\n\r\n\
%s"
#else
#define YX_HTTP_HEADER_CUSTOM  "POST %s HTTP/1.1\r\n\
Content-Length: %d\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Host: 119.29.78.138:30458\r\n\
User-Agent: Fiddler\r\n\
Connection: keep-alive\r\n\r\n\
%s"
#endif
#define YX_COMMAND_STRING_GPS "/v1/accept.vhtml"
#define YX_COMMAND_STRING_TIME "/v1/time.vhtml"

#define YX_JSON_KEY_IMEI        "imei"
#define YX_JSON_KEY_GPS_LAT        "gps_lat"
#define YX_JSON_KEY_GPS_LNG        "gps_lng"
#define YX_JSON_KEY_NO_GPS_DATA        "0"
#define YX_JSON_KEY_CELL_HEAD        "lbs"
#define YX_JSON_KEY_CELL_ID        "cellid"
#define YX_JSON_KEY_CELL_LAC        "lac"
#define YX_JSON_KEY_CELL_MCC        "mcc"
#define YX_JSON_KEY_CELL_MNC        "mnc"
#define YX_JSON_KEY_CELL_SIGNAL        "signal"
#define YX_JSON_KEY_TIME        "time"
#define YX_JSON_KEY_UPLOAD_TYPE        "type"


#define JWCLOUD_COMMAND_STRING_UPLOAD_LOCATION       "/watch/location"
#ifdef __CENON_YX_FOREIGN_VERSION__
#define CENON_SOC_DEF_DNS_NAME        "a615.leelvis.com"
#else
#define CENON_SOC_DEF_DNS_NAME        "a615.careeach.net"
#endif
#define YX_PDU_LEN_MAX  (1024)
static char g_pdu_buffer[YX_PDU_LEN_MAX+2] = {0};
static char  imei[YX_IMEI_STR_LEN+1] = {0}; 
//#define __CENON_YX_FOREIGN_VERSION__ MMI_TRUE
/*========================================================================================================
										T Y P E D E F
========================================================================================================*/
typedef enum
{
	UNKNOW_SHUTDOWN 			= 0x00,
	USER_SHUTDOWN 				= 0x01,		//ʹ���߹ػ�	 
	LOW_BATTERY_SHUTDOWN 		= 0x02,		//�͵����ػ�
	ALARM_TIME_SHUTDOWN 		= 0x03,		//��ʱ�ػ�
}SHUT_DOWN_TYPE;  						// �ػ�����

typedef enum 
{
	MSG_NONE							= 0x00,
		
	//������ϢID
	MSG_WATCH_INFO 						= 0x92,    
	MSG_SHUTDOWN 						= 0x93,
	MSG_LOCATE 							= 0x8C,
	MSG_SOS 							= 0x94,
	MSG_TIWEN 							= 0x9F,    
	MSG_XUEYANG 						= 0x9C,
	MSG_XUETANG 						= 0x97,
	MSG_XUEYA 							= 0x9A,
	MSG_TIZHI 							= 0x98, 
	MSG_MEDICATE_REMINDER_REPLY 		= 0xBA,
	MSG_MEASURE_REMINDER_REPLY			= 0xBB,
	MSG_EMERGENCY_NUMBER_SET_REPLY		= 0xBC,
	MSG_POSITION_START_COMMAND_REPLY	= 0xA1,
	MSG_POSITION_END_COMMAND_REPLY		= 0xA2,
	MSG_HEART_BEAT						= 0xA3,

	//������ϢID
	MSG_WATCH_INFO_REPLY 				= 0x92,    
	MSG_LOCATE_REPLY 					= 0x8C,
	MSG_SOS_REPLY 						= 0x94,
	MSG_TIWEN_REPLY 					= 0x9F,    
	MSG_XUEYANG_REPLY 					= 0x9C,
	MSG_XUETANG_REPLY 					= 0x97,
	MSG_XUEYA_REPLY 					= 0x9A,
	MSG_TIZHI_REPLY 					= 0x98,    
	MSG_MEDICATE_REMINDER 				= 0xBA,
	MSG_MEASURE_REMINDER 				= 0xBB,
	MSG_EMERGENCY_NUMBER_SET 			= 0xBC,
	MSG_POSITION_START_COMMAND			= 0xA1,
	MSG_POSITION_END_COMMAND			= 0xA2,
	MSG_HEART_BEAT_REPLY				= 0xA3,
}PACKET_MSG_ID;
/*========================================================================================================
										V A R I A B L E S
========================================================================================================*/
ST_GPS_SOCKET_CNTX g_qiwo_socket_cntx={0,0,-1};
BOOL watch_info_send_success = FALSE; 
SHUT_DOWN_TYPE shut_down_type = UNKNOW_SHUTDOWN;
kal_uint8 g_connect_count = 0;

BOOL g_gps_soc_recv_idle=TRUE;		// �Ƿ���Խ���һ�� ����
U8  g_gps_soc_recv_buff[1024];		//�ӷ�����ÿ�ν������ݰ���buffer
U8 g_gps_cell_info[2048] = {0};
//GPS_CELL_INFO g_gps_jizhan[PAYLOAD_MAX_CELL_COUNT] = {0};


extern void MDrv_AGPS_Socket_Event_Register(BOOL reg);
//extern void yx_print_apn_infor(void);

/*========================================================================================================
										F U N C T I O N
========================================================================================================*/
void MDrv_Platform_Str_Mass_Print_ex(const u8* buf, u32 len) 
{
	U16 i=0;
	U8 sub_len=50;
	U16 seg=len/sub_len;
	
	for(i=0; i<seg; i++)	
	{
		kal_prompt_trace(QIWO_MOD, "sub_idx=%d, len=%d, buff=%50s. \n", i, sub_len, buf+sub_len*i);
	}
	kal_prompt_trace(QIWO_MOD, "sub_idx=%d, len=%d, buff=%s. \n", i, len%sub_len, buf+sub_len*i);
}

void MDrv_Soc_Gps_Socket_After_RecvProcess_Completed(void) // ���ݴ�����Ϻ��ɨβ����
{
	memset(g_gps_soc_recv_buff, 0, sizeof(g_gps_soc_recv_buff));
	g_gps_soc_recv_idle =TRUE;
	kal_prompt_trace(QIWO_MOD,"RRRRRRRRRRRRRRRRRRR MDrv_Soc_Gps_Socket_After_RecvProcess_Completed end \n");
}

static S8 MDrv_Soc_Gps_Socket_Receive(void)  
{
	MMI_BOOL bReady = MMI_FALSE;
	kal_int32 len = 0;
	kal_int32 index=0;
	kal_int32 div_count=0;
	kal_int32 msg_id=0;
	
	DEBUG_SOC_FUNC();
	
	bReady = soc_ready_for_read(g_qiwo_socket_cntx.soc_socket_id);

	if(!bReady)
	{
		kal_prompt_trace(QIWO_MOD,"[soc_recv], soc_ready_for_read is not ready! \n");
	}
	else
	{
		kal_prompt_trace(QIWO_MOD,"[soc_recv], soc_ready_for_read is ready! \n");
	//-----------------------------------------------
	/*
		// ���Ѿ������ϵķ������Ͻ�������
		// ͬ�������ģʽ 3 ��ʹ�ã����յ���Ϣ�пɶ�����ʱ��
		// ���Ե��øú�������socket �����ȡ����
		// ����ͬ��
		kal_int32 soc_recv(kal_int8  s,
	                       void *buf,
	                       kal_int32 len,
	                       kal_uint8 flags);
	*/
		len = soc_recv(g_qiwo_socket_cntx.soc_socket_id, g_gps_soc_recv_buff, PACKAGE_BUFFER_LEN, 0);
		kal_prompt_trace(QIWO_MOD,"soc_recv Recvie len=%d.\n", len);
		
	//-----------------------------------------------
		if(g_gps_soc_recv_idle)	// һ���µ�ָ�����	
		{
			g_gps_soc_recv_idle = FALSE;
			kal_prompt_trace(QIWO_MOD,"fisrt recv.\n");
			kal_prompt_trace(QIWO_MOD,"fisrt recv.\n");
			kal_prompt_trace(QIWO_MOD,"fisrt recv.\n");
		#if defined(__CENON_DEBUG_SUPPORT__)
			// ��ӡ�������ݣ�len����30�Ļ����ʹ�ӡ30���ֽ�
			//MDrv_Platform_Buffer_Print("print fist", g_gps_soc_recv_buff, CENON_MIN(ONCE_PRINT_LEN, len));
		#endif
		// �жϵ�ǰָ��
			msg_id = *(g_gps_soc_recv_buff+PACKET_DATA_HEADER_LEN+PACKAGE_LENTH_LEN);
			MDrv_Soc_Gps_Protocol_Print_MsgId(msg_id,1);
		}
		else
		{
			kal_prompt_trace(QIWO_MOD,"next recv.\n");
			kal_prompt_trace(QIWO_MOD,"next recv.\n");
			kal_prompt_trace(QIWO_MOD,"next recv.\n");
		}
	//-----------------------------------------------
		
		MDrv_Soc_Gps_Protocol_Recv(g_gps_soc_recv_buff, len);
		kal_prompt_trace(QIWO_MOD,"process one receive OK OK OK OK OK OK .\n");	
		kal_prompt_trace(QIWO_MOD,"process one receive OK OK OK OK OK OK .\n");	
		kal_prompt_trace(QIWO_MOD,"process one receive OK OK OK OK OK OK .\n");	
		// ɨβ����		
		MDrv_Soc_Gps_Socket_After_RecvProcess_Completed();
		
	//-----------------------------------------------
		MDrv_Soc_Gps_Protocol_ReSend_Or_Reply();//�������·������ݣ���Ҫ�ذ��ظ��Ƿ���ճɹ�
	}

	return 0;
}

S32 MDrv_Soc_Gps_Socket_Send(PACKET_PDU * pdu)	 
{	
	S32 ret=0;
	U8 send_fail_count=0;
	kal_int32 msg_id=0;
	
	DEBUG_SOC_FUNC();
	
	kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Socket_Send start \n");


/*
	// ���Ѿ�connect ���ϵķ�������������
	// buf �� len����Ҫ���͵����ݺͳ���
	// flags ��ʱ��ʹ��
	// ���Ҳ�Ǻܳ��õģ� �������֮�󣬾Ϳ��Է���������
	// ����Ҫ����һ����ҳ����ô�ͷ���һ��HTTP ����Ϳ�����
	// ��soc_connect һ���������� ģʽ 3 ��ʹ��
	kal_int32 soc_send(kal_int8	 s,
                       void       *buf,
                       kal_int32  len,
                       kal_uint8  flags);
*/
resend:

	if (!pdu) 
		return 0;

	if(pdu->buffer && pdu->len)
	{
		kal_prompt_trace(QIWO_MOD,"pdu->len=%d \n", pdu->len);

		//msg_id =(kal_int32) *(pdu->buffer+PACKET_DATA_HEADER_LEN+PACKAGE_LENTH_LEN);
		//MDrv_Soc_Gps_Protocol_Print_MsgId(msg_id,0);
		
		//MDrv_Platform_Buffer_Print("buff to be send", pdu->buffer, pdu->len);
		ret = soc_send(g_qiwo_socket_cntx.soc_socket_id, pdu->buffer, pdu->len, 0);
		
	//-------------------------------
		if(ret>0)
		{
			kal_prompt_trace(QIWO_MOD,"soc_send [OK], s32_ret=%d \n", ret);
			if(ret==pdu->len)
			{
				kal_prompt_trace(QIWO_MOD,"one packet send completed \n");
			}
			else
			{
				kal_prompt_trace(QIWO_MOD,"one packet send uncompleted----ret=%d, pdu->len=%d.\n", ret, pdu->len);
			}
		}
		else
		{
			goto send_error;
		}
	//-------------------------------
	}
	
	kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Socket_Send end \n");

	return ret;
	
send_error:
	send_fail_count++;

	if(send_fail_count>=3)
	{
		kal_prompt_trace(QIWO_MOD,"resend out of counts, send_fail_count=%d \n", send_fail_count);
		kal_prompt_trace(QIWO_MOD,"resend out of counts, send_fail_count=%d \n", send_fail_count);
		kal_prompt_trace(QIWO_MOD,"resend out of counts, send_fail_count=%d \n", send_fail_count);
		//MDrv_Soc_Gps_Socket_ReConnect(LIST_RECONNECT_CAUSE_SEND_FAIL);
		MDrv_Soc_Gps_Socket_ReConnect(reconnect_and_send_data_type);
	}
	else
	{
		kal_prompt_trace(QIWO_MOD,"prepare to resend.\n");
		kal_prompt_trace(QIWO_MOD,"prepare to resend.\n");
		kal_prompt_trace(QIWO_MOD,"prepare to resend.\n");
		goto resend;
	}

	return ret;
}

static S8 MDrv_Soc_Gps_Socket_Close(kal_int8 socket_id)	 
{
	S8 ret=0;   
	
	DEBUG_SOC_FUNC();
	
	ret = soc_close(socket_id);

	if(ret == SOC_SUCCESS)
		kal_prompt_trace(QIWO_MOD,"soc_close success!");
	else
		kal_prompt_trace(QIWO_MOD,"soc_close fail! ret=%d", ret);

	g_qiwo_socket_cntx.soc_socket_id = -1;
	g_qiwo_socket_cntx.connect_success =FALSE;
	
	return ret;
}

static void MDrv_Soc_Gps_Check_Connect_Timer(void)// ����Ƿ����ӳɹ�
{
	DEBUG_SOC_FUNC();	
	kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Check_Connect_Timer \n");

	if(g_qiwo_socket_cntx.connect_success)
	{
		kal_prompt_trace(QIWO_MOD,"g_qiwo_socket_cntx.connect_success=%d \n", g_qiwo_socket_cntx.connect_success);
		StopTimer(TIMER_ID_SOCKET_GPS_CHECK_CONNECT);
	}
	else
	{
		kal_prompt_trace(QIWO_MOD,"g_qiwo_socket_cntx.connect_success=%d \n", g_qiwo_socket_cntx.connect_success);
		StopTimer(TIMER_ID_SOCKET_GPS_CHECK_CONNECT);
		MDrv_Soc_Gps_Socket_ReConnect(reconnect_and_send_data_type);
	}
}

void get_gps_body(void)
{
    cJSON *root = NULL;
    char* body_pdu = NULL;
    cJSON* points_array =  NULL;
    J_GPS_DATA gps_data= {0};
    J_CELL_DATA_ARRAY cell_data = {0};
    int i = 0;
    char temp[64] ={0};
    cJSON* points_item = NULL;
    double fLon = 0;
    double fLat= 0;
    int  timezone = (int )100* cenon_func_datetime_get_timezone();
    kal_prompt_trace(QIWO_MOD,"get_gps_body  IN");
    jwsdk_device_read_cell_data(&cell_data);
    jwsdk_device_read_gps_data(&gps_data);

    root = cJSON_CreateObject();

    if(MMI_TRUE ==  jwsdk_device_read_imei(imei, YX_IMEI_STR_LEN))
    {
        cJSON_AddStringToObject(root, YX_JSON_KEY_IMEI, imei);
    }

    if(strlen(gps_data.latitude) && strlen(gps_data.longitude))
    {
        jwsdk_device_get_gps_double(&fLon, &fLat);
        cJSON_AddNumberToObject(root, YX_JSON_KEY_GPS_LAT, fLat);
        cJSON_AddNumberToObject(root, YX_JSON_KEY_GPS_LNG, fLon);
    }
    else
    {
        cJSON_AddStringToObject(root, YX_JSON_KEY_GPS_LAT, YX_JSON_KEY_NO_GPS_DATA);
        cJSON_AddStringToObject(root, YX_JSON_KEY_GPS_LNG, YX_JSON_KEY_NO_GPS_DATA);
    }

    points_array = cJSON_CreateArray();
    cJSON_AddItemToObject(root,YX_JSON_KEY_CELL_HEAD, points_array);
    for(i = 0; i < cell_data.count; i++)
    {
        points_item = NULL;
        points_item = cJSON_CreateObject();
        cJSON_AddNumberToObject(points_item, YX_JSON_KEY_CELL_MCC, cell_data.data_array[i].mcc);
        cJSON_AddNumberToObject(points_item, YX_JSON_KEY_CELL_MNC, cell_data.data_array[i].mnc);
        cJSON_AddNumberToObject(points_item, YX_JSON_KEY_CELL_LAC, cell_data.data_array[i].lac);
        cJSON_AddNumberToObject(points_item, YX_JSON_KEY_CELL_ID, cell_data.data_array[i].ci);
        cJSON_AddNumberToObject(points_item, YX_JSON_KEY_CELL_SIGNAL, cell_data.data_array[i].rssi);
        cJSON_AddItemToArray(points_array,points_item);
    }
    cJSON_AddNumberToObject(root, YX_JSON_KEY_UPLOAD_TYPE, get_yx_upload_type());
    
    if(strlen(gps_data.latitude) && strlen(gps_data.longitude))
    {
        cJSON_AddNumberToObject(root, YX_JSON_KEY_TIME, gps_data.time_stamp_v);
    }
    else
    {
        cJSON_AddNumberToObject(root, YX_JSON_KEY_TIME, cell_data.time_stamp_v);
    }
    
    body_pdu = (char*)cJSON_PrintUnformatted(root);
    
    strcpy(g_pdu_buffer,body_pdu);
    MDrv_Platform_Str_Mass_Print_ex(g_pdu_buffer,strlen(g_pdu_buffer)+1);
    kal_prompt_trace(QIWO_MOD,"get_gps_body  OUT");
    cJSON_FreeText(body_pdu);
    cJSON_Delete(root);
    return;
}

static void MDrv_Soc_Gps_Connected_Aciton(void)
{
    kal_int32 record_locate_total_size_bytes = 0;
    PACKET_PDU packet_pdu = {0};
    U16 package_length = 0;

    kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Connected_Aciton  IN\n");
    
    
    g_qiwo_socket_cntx.connect_success = TRUE;
    kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Connected_Aciton  LIST_RECONNECT_CAUSE_MSG_LOCATE\n");
	reconnect_and_send_data_type = LIST_RECONNECT_CAUSE_MSG_LOCATE;
    if(get_yx_if_need_update_time()){
        sprintf(packet_pdu.buffer, YX_HTTP_HEADER_CUSTOM, YX_COMMAND_STRING_TIME,
          strlen(""),
          "");
    }else{
        get_gps_body();
        sprintf(packet_pdu.buffer, YX_HTTP_HEADER_CUSTOM, YX_COMMAND_STRING_GPS,
          strlen(g_pdu_buffer?g_pdu_buffer:""),
          g_pdu_buffer?g_pdu_buffer:"");
    }
    packet_pdu.len = strlen(packet_pdu.buffer) + 1;
    kal_prompt_trace(QIWO_MOD,"MSG_LOCATE: packet_pdu->len = %d\n",packet_pdu.len);
    MDrv_Platform_Str_Mass_Print_ex(packet_pdu.buffer,packet_pdu.len);
    MDrv_Soc_Gps_Socket_Send(&packet_pdu);
}

void MDrv_Soc_Gps_Socket_Resend(void)
{
	kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Socket_Resend ");
	MDrv_Soc_Gps_Connected_Aciton();
}

static int MDrv_Soc_Gps_Socket_Entry(void);
U8 MDrv_Soc_Qiwo_Socket_Event_Handle(void *msg_ptr)
{
	int s32_ret=0;
	app_soc_notify_ind_struct *soc_notify = (app_soc_notify_ind_struct*) msg_ptr;
	
	DEBUG_SOC_FUNC();
	kal_prompt_trace(QIWO_MOD,"[Event Handler]:g_qiwo_socket_cntx.soc_socket_id=%d \n", g_qiwo_socket_cntx.soc_socket_id);
	kal_prompt_trace(QIWO_MOD,"[Event Handler]:soc_notify->socket_id=%d \n", soc_notify->socket_id);
	kal_prompt_trace(QIWO_MOD,"[Event Handler]:soc_notify->result=%d (1:OK, 0:NG)\n", soc_notify->result);  // note: soc_notify->result =1 is ok
	if(g_qiwo_socket_cntx.soc_socket_id != soc_notify->socket_id)
    {
        kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Qiwo_Socket_Event_Handle g_qiwo_socket_cntx.soc_socket_id != soc_notify->socket_id");
        return;
    }   
    switch (soc_notify->event_type) 
    {
        case SOC_WRITE:		
			// SOC_WRITE ��ʾ����д��Ҳ���ǿ���ͨ��soc_send����������
			kal_prompt_trace(QIWO_MOD,"[Event Handler]:soc_notify->event_type = SOC_WRITE. \n");
			if(soc_notify->result!=0)
			{
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_WRITE ok\n");
			}
			else
			{
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_WRITE error!\n");
				goto event_error_handler;
			}
            break;

        case SOC_READ:  
			// SOC_READ ��ʾ�����ݿɶ���Ҳ���ǿ��Ե��� soc_recv ��ȡ
        	kal_prompt_trace(QIWO_MOD,"[Event Handler]:soc_notify->event_type = SOC_READ. \n");
            is_gps_upload =  0;
			if(soc_notify->result!=0)
			{
				int len = 0;
				int i = 0;
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_READ ok\n");
				kal_prompt_trace(QIWO_MOD,"RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR start \n");

				StopTimer(TIMER_ID_QIWO_GPS_RESEND);
                //yx_set_calibration_status(YX_LOCATION_STATUS_SUCCESS);
				//MDrv_Soc_Gps_Socket_Receive();
				memset(g_gps_cell_info, 0x00, sizeof(U8)*2048);
				len = soc_recv(g_qiwo_socket_cntx.soc_socket_id, g_gps_cell_info, 2048, 0);
				DEBUG_SOC_INFO("soc_recv Recvie len=%d.\n", len);
				MDrv_Platform_Str_Mass_Print_ex(g_gps_cell_info,len+1);
                if(get_yx_if_need_update_time()){
                    yx_set_time(g_gps_cell_info);
                }else{
                    yx_set_upload_interval(g_gps_cell_info);
                }
				memset(g_gps_cell_info, 0x00, sizeof(U8)*2048);
				
				MDrv_Soc_Gps_Socket_Close(g_qiwo_socket_cntx.soc_socket_id);
			}
			else
			{
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_READ error!\n");
				goto event_error_handler;
			}
            break;
            
        case SOC_CONNECT:  
			// SOC_CONNECT ��ʾ�����Ƿ�ɹ�
			kal_prompt_trace(QIWO_MOD,"[Event Handler]:soc_notify->event_type = SOC_CONNECT. \n");	
			if(soc_notify->result != 0)
			{
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CONNECT ok.g_connect_count=%d\n", g_connect_count); 
				MDrv_Soc_Gps_Connected_Aciton();

				StopTimer(TIMER_ID_QIWO_GPS_RESEND);
				StartTimer(TIMER_ID_QIWO_GPS_RESEND, 30*1000, MDrv_Soc_Gps_Socket_Resend);
  
			}
			else
			{
				//kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CONNECT error!\n");
				//kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CONNECT error!\n");
				//kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CONNECT error!\n");
				//kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CONNECT error!\n");
				g_connect_count++;
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CONNECT error!g_connect_count=%d\n", g_connect_count);
				if( g_connect_count<=3 )
				{

					StopTimer(TIMER_ID_QIWO_GPS_RECONNECT);
					StartTimer(TIMER_ID_QIWO_GPS_RECONNECT, 10*1000, MDrv_Soc_Gps_Socket_Entry);	

				}
                else
                {
                    is_gps_upload =  0;
                    yx_set_calibration_status(YX_CALIBRATION_STATUS_CANNOT_CONNECT);
                }
				//MDrv_Soc_Gps_Socket_ReConnect(LIST_RECONNECT_CAUSE_CONNECT_FAIL);
				//goto event_error_handler;
			}
			break;
			
        case SOC_CLOSE:		
			// SOC_CLOSE ��ʾ�Ƿ񱻹رգ� ��������Ҳ�����ǹر����ӵ�
			kal_prompt_trace(QIWO_MOD,"[Event Handler]:soc_notify->event_type = SOC_CLOSE. \n");
			if(soc_notify->result != 0)
			{
			    is_gps_upload =  0;
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CLOSE ok.\n");   
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CLOSE ok.\n");   
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CLOSE ok.\n");   
			}
			else
			{
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CLOSE error!\n");
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CLOSE error!\n");
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_CLOSE error!\n");
				//MDrv_Soc_Gps_Socket_ReConnect(LIST_RECONNECT_CAUSE_OFFLINE);
				MDrv_Soc_Gps_Socket_ReConnect(reconnect_and_send_data_type);
			}
            break;

        case SOC_ACCEPT:
			kal_prompt_trace(QIWO_MOD,"[Event Handler]:soc_notify->event_type = SOC_ACCEPT. \n");	
			if(soc_notify->result != 0)
			{
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_ACCEPT ok.\n");   
			}
			else
			{
				kal_prompt_trace(QIWO_MOD,"[Event Handler]:SOC_ACCEPT error!\n");
				goto event_error_handler;
			}
            break;

 		default:
			kal_prompt_trace(QIWO_MOD,"[Event Handler]:soc_notify->event_type ---goto default. \n");
			break;
	}
	
	return s32_ret;

event_error_handler:
	return s32_ret;
}

BOOL MDrv_Soc_Gps_Device_Get_GPS_OnOff(void)
{
	return MDrv_Soc_Device_GPS_Get_OnOff();
}

void MDrv_Soc_Gps_Device_Set_GPS_OnOff(BOOL OnOff)
{
    if(MDrv_Soc_Device_GPS_Get_OnOff()==OnOff)
    {
        DEBUG_SOC_WARN("current status is eque to set");
        DEBUG_SOC_WARN("current status is eque to set");
        DEBUG_SOC_WARN("current status is eque to set");
        DEBUG_SOC_WARN("current status is eque to set");
        DEBUG_SOC_WARN("current status is eque to set");
        return;
    }
    
	if(OnOff)
	{
		//MDrv_Soc_Jd_Device_Led_Set_Effect(LIST_JD_LED_EFFECT_TYPE_GPS_UNLOCATED);
	}
	else
	{
		//MDrv_Soc_Jd_Device_Led_Set_Effect(LIST_JD_LED_EFFECT_TYPE_GPS_PWROFF);
	}

	MDrv_Soc_Device_GPS_Set_OnOff(OnOff);
}

/*
	//�ر�socket �� soc_create �ɶ�ʹ��
		kal_int8 soc_close(kal_int8 s)

	// ��һ��socket �� һ��ָ����ip��ַ �� �˿ڡ���Ҫ���ڷ������̿���
	// һ��ip ��ַȫ 0���˿ھ�����Ҫ�󶨵Ķ˿�
	// ��ʵ�ʿ����У����������õ������������ֻ�����������hoho��
		kal_int8 soc_bind(kal_int8 s, sockaddr_struct *addr);

	// ����socket ֮�󣬾Ϳ��Լ������ ip�Ķ˿�
	// ���Ҳ���������ã�Ҳ�����ڷ������ˡ�
	// backlog  ͬʱ�����ӵ�socket ����
		kal_int8 soc_listen(kal_int8 s, kal_uint8 backlog);

	// ����socket ���ӵ��û�ʱ��accept �Ϳ��Ի�ø����ӣ�ͬ��Ҳ�����ڷ�������
	// addr ���Ի�����ӹ�����socket ��ַ
	// ����һ���µ�socket 
		kal_int8 soc_accept(kal_int8 s, sockaddr_struct *addr);

	// ��ָ����ip��ַ�Ͷ˿ڷ�������
	// buf �� len �ֱ���Ҫ���͵����ݺͳ���
	// flag ��ʱû��ʹ�� ����Ϊ 0
	// addr ָ��Ҫ���͵���ip��ַ�Ͷ˿�
	// ˵�������create socket ��ʱ����TCP����ô������Ҫ��connect �� ���������ٵ��øú���
	// ���� berkeley �� socket �淶�е����� 
		kal_int32 soc_sendto(kal_int8	     s,
	                         void            *buf,
	                         kal_int32	     len,
	                         kal_uint8 	     flags,
	                         sockaddr_struct *addr);

	// ��ָ����ַ��������
	// buf ��ȡ���ݵ�buf��len ��ȡ����buf����󳤶�
	// flags ��ʱ���ã�����Ϊ0
	// ����ʵ�ʶ�ȡ�����ݳ���
	kal_int32 soc_recvfrom(kal_int8        s,
	                              void            *buf,
	                              kal_int32       len,
	                              kal_uint8       flags,
	                              sockaddr_struct *fromaddr);
	// ͨ���������ip��ַ
	// is_blocking���Ƿ�����������ֻ֧��none block��Ҳ�����none block
	// mod_id ���û��������ã���ô����ѯ����ϣ������mod ������Ϣ
	// request_id ���ֲ�ͬ��DNS ��ѯ�����������ͬʱ��ѯ�������ϣ��ڷ��ص���Ϣ�У��Ϳ���ͨ��id���������֣�����������һ����ѯ���
	// addr ���ֱ�Ӳ�ѯ���������������cache����ôip��ֱַ�ӷ���
	// len ���ص�ip��ַ����
	// access_id Ҳ����ڲ�ѯ������Ϣ���棬����֪����ʲô��
	// nwk_account_id �����
	kal_int8 soc_gethostbyname(kal_bool is_blocking,
	                           module_type     mod_id,
	                           kal_int32       request_id,
	                           const kal_char  *domain_name,
	                           kal_uint8       *addr,
	                           kal_uint8       *addr_len,
	                           kal_uint8       access_id,
	                           kal_uint32      nwk_account_id);
*/
static int MDrv_Soc_Gps_Socket_Entry(void)
{
	int s32_ret=0;
	sockaddr_struct soc_addr;
	char val=0;
    S16 acc_id = (-1);
    char len_addr=0;
    
	kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Socket_Entry IN");
	DEBUG_SOC_FUNC();
//------------------------------------------
#if 1
#ifdef __XI_SUPPORT__
	return;
#endif
	if(g_qiwo_socket_cntx.soc_socket_id > (-1))
	{
		kal_prompt_trace(QIWO_MOD,"close previous g_qiwo_socket_cntx.soc_socket_id.\n");
		MDrv_Soc_Gps_Socket_Close(g_qiwo_socket_cntx.soc_socket_id);		
	}
#endif

	MDrv_Soc_Gps_Socket_Init_Socket_Cnxt();
    kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Socket_Entry 1111111");
//------------------------------------------
	if(!MDrv_Soc_Device_Sim_Get_Valid())
	{
		kal_prompt_trace(QIWO_MOD,"please insert sim card!\n");
		
		if(MDrv_Soc_Gps_Device_Get_GPS_OnOff())//û�忨����Ҫ����ʱ�ر�GPS
		{
			kal_prompt_trace(QIWO_MOD,"GPS IS ON, Close GPS!\n");
			MDrv_Soc_Gps_Device_Set_GPS_OnOff(FALSE);
		}
        yx_set_chick_complete(1);
		yx_set_calibration_status(YX_CALIBRATION_STATUS_NO_SIM_CARD);
		return -1;
	}
	else
	{
		kal_prompt_trace(QIWO_MOD,"sim card is ready.\n");
	}

//------------------------------------------
	s32_ret = cbm_register_app_id(&g_qiwo_socket_cntx.soc_app_id);
	if (s32_ret == CBM_OK)
	{
		kal_prompt_trace(QIWO_MOD,"CBM_OK g_qiwo_socket_cntx.soc_app_id=%d \n", g_qiwo_socket_cntx.soc_app_id); //the valid app_id is obtained, my g_qiwo_socket_cntx.soc_app_id=5
	}
	else
	{
		kal_prompt_trace(QIWO_MOD,"!!!!CBM_OK g_qiwo_socket_cntx.soc_app_id=%d \n", g_qiwo_socket_cntx.soc_app_id);
		goto init_error_handler;
	}
//------------------------------------------
    acc_id = MDrv_Soc_Get_AccountId();
#if 0
    if(acc_id > 0)
	{
		g_qiwo_socket_cntx.soc_account_id = cbm_encode_data_account_id(acc_id, CBM_SIM_ID_SIM1, g_qiwo_socket_cntx.soc_app_id, KAL_FALSE);  
	}
	else
#endif        
	{
		g_qiwo_socket_cntx.soc_account_id = cbm_encode_data_account_id(CBM_DEFAULT_ACCT_ID, CBM_SIM_ID_SIM1, g_qiwo_socket_cntx.soc_app_id, KAL_FALSE);  
	}

    kal_prompt_trace(QIWO_MOD,"g_qiwo_socket_cntx.soc_account_id=%d \n", g_qiwo_socket_cntx.soc_account_id);
//------------------------------------------
/*
	mmi_frm_set_protocol_event_handler(MSG_ID_APP_SOC_NOTIFY_IND, MDrv_Socket_Event_Handler, MMI_TRUE);	 
*/
    //MDrv_AGPS_Socket_Event_Register(MMI_TRUE);
//------------------------------------------
/*
*    ����һ��socket
*    domain��    Э�飬����ֻ֧�� SOC_PF_INET
*    type ��      ��domain �µ����ͣ������� SOC_SOCK_STREAM(TCP),SOC_SOCK_DGRAM(UDP),SOC_SOCK_SMS,SOC_SOCK_RAW
*    protocol��   Э�����ͣ�����type Ϊ SOC_SOCK_RAW��������Ϊ 0
*    mod_id��   ��ǰģ��ID,��Ҫ���ڽ�����Ϣ
*    nwk_account_id��apn �����
*/
 	g_qiwo_socket_cntx.soc_socket_id = soc_create(SOC_PF_INET, SOC_SOCK_STREAM, 0, MOD_MMI, g_qiwo_socket_cntx.soc_account_id );    
 	if(g_qiwo_socket_cntx.soc_socket_id<0)
	{
		kal_prompt_trace(QIWO_MOD,"g_qiwo_socket_cntx.soc_socket_id=%d \n", g_qiwo_socket_cntx.soc_socket_id);  
		goto init_error_handler;
	}
	else
	{
		kal_prompt_trace(QIWO_MOD,"g_qiwo_socket_cntx.soc_socket_id=%d \n", g_qiwo_socket_cntx.soc_socket_id);
	}
//------------------------------------------
/*
	MTK socket ��Ҫ������ģʽ��block(����)��non-block(������)��Asynchronous(�첽)��
	��Ϸ�ʽ Ҳֻ������ 1 block ��2 non-block��3 non-block + Asynchronous��

	blockģʽ��:
			������Ӧ�ĺ��������ܻ��߷������ݣ�����������������û�����(û�з��ͻ��߽������)��
			��ô�����Ͳ��ᷴ�أ���ô���õ�����task���ͻ����������в����κζ����������MMI MOD 
			����ֱ������������Ǻ�Σ�յģ�������ֻ�û����Ӧ���ּ����������Լ����������ģʽ��

	non-block ģʽ�£�
			������Ӧ�ĺ��������ܷ���ok����block���󲿷�����·���block����ʾ���ݻ�û�д�����ϣ�
			���Ǻ������������ء�����ʲôʱ���ʾ���ݴ�������أ���Ҳ��һ���Ƚ�ͷ�۵����顣���
			ʱ��Ҫ���select������һ��ʹ�ã���������Ҫ�Լ���ѯȥ��ѯ��Ӧ��socket�Ƿ����ʹ���ˡ�
			һ��Ҳ���ã�Ч�ʱȽϵ͡�

	non-block + Asynchronousģʽ��
			���ģʽ�Ƽ�ʹ�ã����Ա�ɹ����м����������ַ�ʽ��non-block���Ͳ���������������Ӧ��
			������������Asynchronousģʽ����ô��ʹ�ú�������blockʱ��app ֻҪע����Ӧ�Ļص�������
			�����ݴ�������ˣ��ͻ��յ���Ӧ��֪ͨ�������Լ�ȥ��ѯ��Ч��Ҳ�͸��ˡ�


	// ����socket�Ĳ�������������socket ֮�󣬾Ϳ���������
	// ģʽ 3 none block + asynchronous ����ͨ������������ö���
			kal_int8 soc_setsockopt(kal_int8   s,
  	                               kal_uint32 option,
  	                               void       *val,
  	                               kal_uint8  val_size);
	// ����
	 S8 val = 0, ret = 0;
	 
	 val = KAL_TRUE;
	 
	 // ����Ϊnone block ģʽ��Ĭ��Ϊblock ģʽ
	 ret = soc_setsockopt(soc_id , SOC_NBIO, &val, sizeof(val));
	 
	 // ����Ϊ�첽ģʽ�����Ҽ�����Ϣ������������ 
	 // SOC_READ ��ʾ�����ݿɶ���Ҳ���ǿ��Ե��� soc_recv ��ȡ
	 // SOC_WRITE ��ʾ����д��Ҳ���ǿ���ͨ��soc_send����������
	 // SOC_CONNECT ��ʾ�����Ƿ�ɹ�
	 // SOC_CLOSE ��ʾ�Ƿ񱻹رգ� ��������Ҳ�����ǹر����ӵ�
	 val = SOC_READ|SOC_WRITE|SOC_CONNECT|SOC_CLOSE;
	 ret = soc_setsockopt(soc_id, SOC_ASYNC, &val, sizeof(val));
*/
	val = KAL_TRUE;	
	s32_ret =soc_setsockopt(g_qiwo_socket_cntx.soc_socket_id , SOC_NBIO, &val, sizeof(val));	
	if(s32_ret<0)
	{
		kal_prompt_trace(QIWO_MOD,"soc_setsockopt(SOC_NBIO) failed!! s32_ret=%d \n", s32_ret);
		goto init_error_handler;
	}
	else
	{
		kal_prompt_trace(QIWO_MOD,"soc_setsockopt(SOC_NBIO) success!! s32_ret=%d \n", s32_ret);
	}
/*
	// ����Ϊ�첽ģʽ�����Ҽ�����Ϣ������������ 
	// SOC_READ ��ʾ�����ݿɶ���Ҳ���ǿ��Ե��� soc_recv ��ȡ
	// SOC_WRITE ��ʾ����д��Ҳ���ǿ���ͨ��soc_send����������
	// SOC_CONNECT ��ʾ�����Ƿ�ɹ�
	// SOC_CLOSE ��ʾ�Ƿ񱻹رգ� ��������Ҳ�����ǹر����ӵ�
*/
// ���ü����ļ���socket�¼�
	val = SOC_READ|SOC_WRITE|SOC_CONNECT|SOC_CLOSE;  
	s32_ret =soc_setsockopt(g_qiwo_socket_cntx.soc_socket_id, SOC_ASYNC, &val, sizeof(val));	
	if(s32_ret<0)
	{
		kal_prompt_trace(QIWO_MOD,"soc_setsockopt(SOC_ASYNC) failed!! s32_ret=%d \n", s32_ret);
		goto init_error_handler;
	}
	else
	{
		kal_prompt_trace(QIWO_MOD,"soc_setsockopt(SOC_ASYNC) success!! s32_ret=%d \n", s32_ret);
	}
//------------------------------------------
    #if 0 //def __CENON_YX_FOREIGN_VERSION__
    soc_addr.addr[0] = 47;
	soc_addr.addr[1] = 88;
	soc_addr.addr[2] = 6;
	soc_addr.addr[3] = 149;
	soc_addr.port = 30458;
    soc_addr.sock_type =SOC_SOCK_STREAM;
    kal_prompt_trace(QIWO_MOD,"[QIWO soc_connect]----ip:%d.%d.%d.%d, port=%d. \n", soc_addr.addr[0],  soc_addr.addr[1],  soc_addr.addr[2], soc_addr.addr[3], soc_addr.port);
    #else
    kal_prompt_trace(QIWO_MOD,"CENON_SOC_DEF_DNS_NAME=%s \n", CENON_SOC_DEF_DNS_NAME);
    s32_ret = soc_gethostbyname(0, MOD_MMI, g_qiwo_socket_cntx.soc_app_id, CENON_SOC_DEF_DNS_NAME, soc_addr.addr, &len_addr, 0, g_qiwo_socket_cntx.soc_account_id);
    soc_addr.sock_type =SOC_SOCK_STREAM;
    soc_addr.port = 30458;
    #endif
   
	//kal_prompt_trace(QIWO_MOD,"[QIWO soc_connect]----ip:%d.%d.%d.%d, port=%d. \n", soc_addr.addr[0],  soc_addr.addr[1],  soc_addr.addr[2], soc_addr.addr[3], soc_addr.port);
	switch(soc_addr.sock_type)
	{
		case SOC_SOCK_STREAM:	kal_prompt_trace(QIWO_MOD,"soc_addr.sock_type = SOC_SOCK_STREAM.\n");	break;
		case SOC_SOCK_DGRAM:		kal_prompt_trace(QIWO_MOD,"soc_addr.sock_type = SOC_SOCK_DGRAM.\n");	break;
		case SOC_SOCK_SMS:		kal_prompt_trace(QIWO_MOD,"soc_addr.sock_type = SOC_SOCK_SMS.\n");		break;
		case SOC_SOCK_RAW:		kal_prompt_trace(QIWO_MOD,"soc_addr.sock_type = SOC_SOCK_RAW.\n");		break;
		default:					kal_prompt_trace(QIWO_MOD,"soc_addr.sock_type = unknown.\n");			break;
	}
//------------------------------------------
/*
	// ���ӵ�һ��ָ��ip��ַ�ķ�����
	// ��������ܳ��ã�Ҫ���������͵�ͨ�� addr ָ��ip��ַ�Ͷ˿�
	// ���ݲ�ͬ��ģʽ (block, none block,asynchronous,�����������΢��ͬ
	// �����block����ô����task �ͱ�block��ֱ��connect �ɹ�����ʧ�ܻ��߳�ʱ
	// ��������MMI task ���棬����ͻ�Ƚ����أ����治��������Ӧ��
	// ������� none block + asynchronous,  �������Ϸ��أ��ɹ���ʧ�ܻ���SOC_WOULDBLOCK
	// ������·���SOC_WOULDBLOCK����ô�͵ȴ���Ϣ������Ϣ��֪���׳ɹ�orʧ�ܡ�
	// addr ����Ҫ���ӵ�ip��ַ�Ͷ˿�
	kal_int8 soc_connect(kal_int8 s, sockaddr_struct *addr)
*/
#if 0 //def __CENON_YX_FOREIGN_VERSION__
	kal_prompt_trace(QIWO_MOD,"Do [soc_connect]. %d\n", g_qiwo_socket_cntx.soc_socket_id);
	s32_ret = soc_connect(g_qiwo_socket_cntx.soc_socket_id, &soc_addr);	// when first connect, it nearly return SOC_WOULDBLOCK, and then goto event handler
// ����Ƿ����ӳɹ�
	//StopTimer(TIMER_ID_SOCKET_GPS_CHECK_CONNECT);
	//StartTimer(TIMER_ID_SOCKET_GPS_CHECK_CONNECT, SOC_GPS_RECONNECT_TIME, MDrv_Soc_Gps_Check_Connect_Timer);	 	

    switch (s32_ret)
    {
        case SOC_SUCCESS:
			kal_prompt_trace(QIWO_MOD,"[soc_connect]----ret=SOC_SUCCESS \n");
			MDrv_Soc_Gps_Connected_Aciton();	
			break;
			
        case SOC_WOULDBLOCK:
			kal_prompt_trace(QIWO_MOD,"[soc_connect]----ret=SOC_WOULDBLOCK \n");
            break;

        default:          
			kal_prompt_trace(QIWO_MOD,"[soc_connect]----ret=default, s32_ret=%d \n", s32_ret);
			break;
	}
#endif
//------------------------------------------
    yx_set_chick_complete(1);
	return s32_ret;
	
init_error_handler:
	kal_prompt_trace(QIWO_MOD,"restart to init socket.\n");

	StopTimer(TIMER_ID_SOCKET_GPS_CHECK_CONNECT);
	StartTimer(TIMER_ID_SOCKET_GPS_CHECK_CONNECT, SOC_GPS_REENTRY_TIME, MDrv_Soc_Gps_Socket_Entry);	 

       return s32_ret;
}
/*

    �����շ�           ͨѶ                �����շ�����             �����趨               ��������            ���ؽӿ�				
 ----------           ----------           -------------           --------------           ---------         ---------------		
|  Server  |  <--->  |  Socket  |  <--->  | JD Protocol |  <--->  | JD Funciton  |  <--->  | JD Data | <---> |  Device Data  |
 ----------           ----------           -------------           --------------           ---------         ---------------	
                                                                         |                                            |	
                                                                         |---------------------->---------------------|
                                                                       					          ��������
*/

/*========================================================================================================
										F U N C T I O N	---- public
========================================================================================================*/
void MDrv_Soc_Gps_Socket_Init_Socket_Cnxt(void)
{
	DEBUG_SOC_FUNC();
	
	memset(&g_qiwo_socket_cntx, 0, sizeof(ST_GPS_SOCKET_CNTX));
    g_qiwo_socket_cntx.soc_account_id = 0;
    g_qiwo_socket_cntx.soc_app_id = 0;
    g_qiwo_socket_cntx.soc_socket_id = -1;
}

void MDrv_Soc_Gps_Socket_Entry_Proxy(void)
{
	kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Socket_Entry_Proxy IN");
	g_connect_count = 0;
    is_gps_upload = 1;
    is_gps_searching = 0;
	MDrv_Soc_Gps_Socket_Entry();
}

void MDrv_Soc_Gps_Socket_ReConnect(ST_GPS_RECONNECT_CAUSE cause)  // ��������socket
{	
	U8 i=0;
	
	DEBUG_SOC_FUNC();
	
#if defined(__CENON_DEBUG_SUPPORT__)
	switch(cause)
	{
		case LIST_RECONNECT_CAUSE_CONNECT_FAIL:		for(i=0; i<5; i++){kal_prompt_trace(QIWO_MOD, "LIST_RECONNECT_CAUSE_CONNECT_FAIL.\n");}	break;
		case LIST_RECONNECT_CAUSE_LOGIN_FAIL:		for(i=0; i<5; i++){kal_prompt_trace(QIWO_MOD, "LIST_RECONNECT_CAUSE_LOGIN_FAIL.\n");}		break;
		case LIST_RECONNECT_CAUSE_OFFLINE:			for(i=0; i<5; i++){kal_prompt_trace(QIWO_MOD, "LIST_RECONNECT_CAUSE_OFFLINE.\n");}		break;
		case LIST_RECONNECT_CAUSE_SEND_FAIL:			for(i=0; i<5; i++){kal_prompt_trace(QIWO_MOD, "LIST_RECONNECT_CAUSE_SEND_FAIL.\n");}		break;
		case LIST_RECONNECT_CAUSE_AFTER_CALL:		for(i=0; i<5; i++){kal_prompt_trace(QIWO_MOD, "LIST_RECONNECT_CAUSE_AFTER_CALL.\n");}		break; 
		case LIST_RECONNECT_CAUSE_CFG_IPP:			for(i=0; i<5; i++){kal_prompt_trace(QIWO_MOD, "LIST_RECONNECT_CAUSE_CFG_IPP.\n");}		break;
		case LIST_RECONNECT_CAUSE_NOT_HTB:			for(i=0; i<5; i++){kal_prompt_trace(QIWO_MOD, "LIST_RECONNECT_CAUSE_NOT_HTB.\n");}		break;		 
		case LIST_RECONNECT_CAUSE_MSG_LOCATE:		for(i=0; i<5; i++){kal_prompt_trace(QIWO_MOD, "LIST_RECONNECT_CAUSE_MSG_LOCATE.\n");}		break;
		case LIST_RECONNECT_CAUSE_MSG_SOS:			for(i=0; i<5; i++){kal_prompt_trace(QIWO_MOD, "LIST_RECONNECT_CAUSE_MSG_SOS.\n");}		break;
		default:
			kal_prompt_trace(QIWO_MOD, "[MDrv_Soc_Gps_Socket_ReConnect]----(cause=%d)goto default.\n", cause);
			break;
	}
#endif
	
	MDrv_Soc_Gps_Socket_Entry();
}

void MDrv_Soc_Gps_Send_Info_When_Shut_Down(SHUT_DOWN_TYPE shutdown_type)
{
	MMI_BOOL is_shutdown_type = FALSE;
	
	kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Send_Info_When_Shut_Down start \n");

	switch(shutdown_type)
	{
		case USER_SHUTDOWN://ʹ���߹ػ�
			is_shutdown_type = TRUE;
			shut_down_type = USER_SHUTDOWN;
			break;
			
		case LOW_BATTERY_SHUTDOWN://�͵����ػ�
			is_shutdown_type = TRUE;
			shut_down_type = LOW_BATTERY_SHUTDOWN;
			break;
			
		case ALARM_TIME_SHUTDOWN://��ʱ�ػ�
			is_shutdown_type = TRUE;
			shut_down_type = ALARM_TIME_SHUTDOWN;
			break;
			
		default:
			is_shutdown_type = FALSE;
			shut_down_type = UNKNOW_SHUTDOWN;
			break;
	}

	if (is_shutdown_type)
	{
		if (g_qiwo_socket_cntx.connect_success)
		{
			//MDrv_Soc_Gps_Protocol_Send(MSG_SHUTDOWN);
		}
		else
		{
			reconnect_and_send_data_type = LIST_RECONNECT_CAUSE_MSG_SHUTDOWN;
			MDrv_Soc_Gps_Socket_ReConnect(LIST_RECONNECT_CAUSE_MSG_SHUTDOWN);
		}
	}

	kal_prompt_trace(QIWO_MOD,"MDrv_Soc_Gps_Send_Info_When_Shut_Down end \n");
}

void MDrv_Soc_Gps_Function_SearchCell(void)
{
    U32 time = 0;
    DEBUG_SOC_INFO("MDrv_Soc_Voto_Function_SearchCell");
    MDrv_Soc_Device_Cell_Request();
}

void MDrv_Soc_Gps_Function_Cancel_SearchCell(void)
{
    U32 time = 0;
    DEBUG_SOC_INFO("MDrv_Soc_Voto_Function_Cancel_SearchCell");
    MDrv_Soc_Device_Cell_Cancel();
}

void MDrv_Soc_Gps_Function_GetCellID(AGPS_CELL_DATA* data)
{
   U8 i = 0;
   U8 temp[200] = {0};
    
    kal_prompt_trace(QIWO_MOD, "TIny IN");

    memset(g_gps_cell_info, 0x00, sizeof(U8)*2048);
    
//    for(i = 0; i < PAYLOAD_MAX_CELL_COUNT; i++)
     for(i = 0; i < data->cell_count; i++)
    {
 #if 0
        if(0/* == data->cell_info[i].mcc*/)
        {
            kal_prompt_trace(QIWO_MOD, "TIny cellid ==== %d  return", data->cell_info[i].mcc);
            return;
        }
        else
#endif
        {
            memset(temp, 0x00, sizeof(U8)*200);
            sprintf(temp, "{\"mcc\":%d,\"lac\":%d,\"cellid\":%d,\"mnc\":%d,\"signal\":%d}", data->cell_info[i].mcc
                , data->cell_info[i].lac, data->cell_info[i].cell_id, data->cell_info[i].mnc, data->cell_info[i].rssi);
            if(strlen(g_gps_cell_info))
            {
                strcat(g_gps_cell_info, ",");
            }
            else
            {
                strcpy(g_gps_cell_info,temp);
                continue;
            }
            strcat(g_gps_cell_info, temp);
        }
        kal_prompt_trace(QIWO_MOD, "TIny cell data %d", temp);
        MDrv_Platform_Str_Mass_Print_ex(g_gps_cell_info,strlen(g_gps_cell_info)+1);
        kal_prompt_trace(QIWO_MOD, "TIny cellid %d", data->cell_info[i].cell_id);
        kal_prompt_trace(QIWO_MOD, "TIny lac %d", data->cell_info[i].lac);
        kal_prompt_trace(QIWO_MOD, "TIny mcc %d", data->cell_info[i].mcc);
        kal_prompt_trace(QIWO_MOD, "TIny mnc %d", data->cell_info[i].mnc);
        kal_prompt_trace(QIWO_MOD, "TIny rssi %d", data->cell_info[i].rssi);
        
    }
}
#endif//#if defined(__CENON_SOCKET_QIWO_SUPPORT__)

