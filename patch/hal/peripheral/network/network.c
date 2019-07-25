#include "stack_common.h"  
#include "stack_msgs.h"
#include "app_ltlcom.h"       /* Task message communiction */
#include "syscomp_config.h"
#include "kal_general_types.h"
#include "kal_public_api.h"
#include "kal_public_defs.h"
#include "sw_keypad.h"
#include "stdlib.h"
#include "stdio.h"
#include "soc_api.h"
#include "soc_consts.h"
#include "app2soc_struct.h"
#include "soc2tcpip_struct.h"
#include "mnsms_struct.h"
#include "cbm_api.h"

#include "MMIDataType.h"
#include "MMITimer.h"
#include <TimerEvents.h>  
#include "port.h"
#include "protocol.h"
#include "network.h"
#include "sampling.h"
#include "nvram_interface.h"
#include "nvram_common_defs.h"

uint32_t network_accout_id = 0;
socket_attr_t attr = {URL,PORT,0xffff};
chain_t chain;


 
static void network_connect(void);
static void network_reconnect(void);


void network_ip_port_init()
{
	int16_t error;
	socket_attr_t soc_attr;
	ReadRecord(NVRAM_EF_IP_PORT_LID,1,(void *)&soc_attr,sizeof(socket_attr_t),&error);
	if (soc_attr.ip[0] == 0xff && soc_attr.ip[1] == 0xff && soc_attr.ip[2] == 0xff && soc_attr.ip[3] == 0xff) {
		WriteRecord(NVRAM_EF_IP_PORT_LID,1,&attr,sizeof(socket_attr_t),&error);
	} else {
		memcpy((uint8_t *)&attr,(uint8_t *)&soc_attr,sizeof(socket_attr_t));
	}
	xi_trace("ip:%d,%d,%d,%d",attr.ip[0],attr.ip[1],attr.ip[2],attr.ip[3]);
}

void network_update_ip_port(uint8_t *ip,uint8_t size)
{
	int16_t error;
	if (ip == NULL) {
		xi_trace("ip param error");
		return;
	}
	memcpy(&attr.ip[0],ip,size);
	WriteRecord(NVRAM_EF_IP_PORT_LID,1,&attr,sizeof(socket_attr_t),&error);
	memcpy(&attr.ip[0],ip,size);
	network_reconnect();
}

uint32_t network_get_account_id(void)
{
    uint8_t app_id;
    cbm_app_info_struct info = {0};
	
	info.app_type = DTCNT_APPTYPE_BRW_HTTP | DTCNT_APPTYPE_BRW_WAP;
	info.app_str_id = 0;
	info.app_icon_id = 0;

    if (CBM_OK != cbm_register_app_id_with_app_info(&info, &app_id)) {
        return false;
    }

    if (app_id == 0) {
		return false;
    } else {
		xi_trace("get account id success!");
	}

    network_accout_id = cbm_encode_data_account_id(CBM_DEFAULT_ACCT_ID, CBM_SIM_ID_SIM1, app_id, 0);
	return true;
}

static void network_reconnect(void)
{
	xi_trace("network_reconnect...");
	if (chain.socket_id >= 0) {
		soc_close(chain.socket_id);
	} 
	chain.socket_id = INVALID_SOCKET;
	chain.state = false;
	chain.frame_sended_size = 0;
	chain.frame_size = 0;
	StartTimer(TIMER_ID_RECONNECT,NETWORK_RECONNECT_INTERVAL,network_connect);	
}

static void network_connect(void)
{
	sockaddr_struct addr;
    uint8_t option;
	int32_t result;

	if (chain.state) {
		xi_trace("connect already success!");
		return ;
	}

	if (!cbm_is_account_valid(network_accout_id)) {
		xi_trace("Account id is invalid (0x%08x)", network_accout_id);
		return ;
	}

	if (chain.socket_id >= 0) {
		soc_close(chain.socket_id);
	}

	chain.socket_id = soc_create(SOC_PF_INET, SOC_SOCK_STREAM, 0, MOD_MMI, network_accout_id); 
	if (chain.socket_id < 0) {
		xi_trace("create socket failed!");
		return ;
	}

	option = SOC_READ | SOC_WRITE | SOC_ACCEPT | SOC_CONNECT | SOC_CLOSE;
	soc_setsockopt(chain.socket_id, SOC_ASYNC, &option, sizeof(option));
	option = KAL_TRUE;
	soc_setsockopt(chain.socket_id, SOC_NBIO, &option, sizeof(option));
    addr.sock_type = SOC_SOCK_STREAM;
    addr.addr_len = 4;
    memcpy(addr.addr, attr.ip, 4);
    addr.port = attr.port;

	result = soc_connect((int8_t)chain.socket_id, &addr);
	xi_trace("connect result:%d",result);
	if (result == SOC_SUCCESS) {
		xi_trace("connect success....");
		return ;
	} else if (result < 0 && result != SOC_WOULDBLOCK) {
		xi_trace("connect failed,start reconect");
		network_reconnect();
		return ;
	}
	return;
}

static int32_t network_send_data(uint8_t *data,int32_t size)
{
	int32_t sended_size = 0;
	int32_t send_size = 0;
	int32_t result = 0;

	if (data == NULL || size <= 0) {
		xi_trace("para error!");
		return false;
	}
	//dump(data,size);
	do {
		send_size = (size - sended_size > 512) ? 512 : (size - sended_size); 
		result = soc_send(chain.socket_id,data + sended_size,send_size,0);
		//memcpy(&chain.send_buf[0],data+sended_size,send_size);
		//chain.frame_size = send_size;
		if (result > 0) {
			sended_size += result;
		}
		xi_trace("send result:%d,sended_size :%d",result,sended_size);
		if (result == SOC_WOULDBLOCK) {
			kal_sleep_task(100);
		} else if (result < 0) {
			break;
		}
	} while(sended_size < size);
	xi_trace("network_send_data size:%d,send result:%d",size,sended_size);
	if (result >= size) {
		return true;
	}

	if (result < 0 && result != SOC_WOULDBLOCK) {
		network_reconnect();
		return false;
	}
	return false;
}

int32_t network_send_frame(uint8_t *data,uint32_t size)
{
	chain.frame_size = size;
	chain.frame_sended_size = 0;
	return network_send_data(data,size);
}

void dump(uint8_t *data,int32_t size)
{
	uint8_t buf[8];
	int32_t i,len = 0;

	memset(buf,0,sizeof(buf));

	for (i = 0; i < size;i += len) {
		len = (size - i) > 8 ? 8 : (size - i);
		memset(buf,0,sizeof(buf));
		memcpy(buf,data,len);
		data += len;
		xi_trace("length:%d--%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,",len,buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
	}
}

static int32_t network_recv_data()
{
	int32_t result;

	result = soc_recv(chain.socket_id,&chain.recv_buf[0],BUF_SIZE,0);
	xi_trace("recv msg:%d",result);
	dump(chain.recv_buf,result);
	if (result > 0) {
		if (protocol_parse(&chain.recv_buf[0],result) > 0) {
			xi_trace("recv complete frame...");
		}
	} else if (result < 0 && result != SOC_WOULDBLOCK) {
		network_reconnect();
	}
}
 
static uint8_t network_soc_notify(void *msg)
{
	app_soc_notify_ind_struct *notify = (app_soc_notify_ind_struct *)msg;

	if (NULL == notify) {
		return false;
	}
	xi_trace("notify->socket_id:%d,chain.socket_id:%d",notify->socket_id,chain.socket_id);
	xi_trace("event_type:%d,result:%d",notify->event_type,notify->result);


	if (notify->socket_id != chain.socket_id) {
		xi_trace("socket id not pair!");
		return false;
	}

	switch (notify->event_type) {
		case SOC_CONNECT:
			if (notify->result) {
				xi_trace("connect success.......");
				chain.state = true;
				sampling();
			} else {
				network_reconnect();
			}
			break;

		case SOC_WRITE:
			if (notify->result) {
				//if (chain.frame_sended_size < chain.frame_size) {
				//network_send_data(&chain.send_buf[0],chain.frame_size);
			//	}
			}
			break;

		case SOC_READ:
			if (notify->result) {
				network_recv_data();
			}
			break;
		case SOC_CLOSE:
			network_reconnect();
			break;
		default:
			break;

	}
	return true;
}

static void network_chain_init(void)
{
	memset(&chain,0,sizeof(chain_t));
	chain.socket_id = INVALID_SOCKET;
	mmi_frm_set_protocol_event_handler(MSG_ID_APP_SOC_NOTIFY_IND,network_soc_notify,MMI_TRUE);
	srv_imei_get_imei(1,&chain.module_id[0],MODULE_ID_LEN+1);
	xi_trace("module_id:%s",chain.module_id);
}

static int32_t network_init(void)
{
	mem_init();
	network_ip_port_init();
	network_chain_init();
	network_get_account_id(); 
}

void network_get_module_id(uint8_t *module_id)
{
	if (module_id != NULL) {
		memcpy(module_id,&chain.module_id[0],MODULE_ID_LEN);
	}
}

void network_service_update(kal_uint8 state)
{
	xi_trace("gsm state:%d",state);
	if (state == 3) {
		if (chain.state == 0) {
			network_init();
			network_connect();
		} 
	}
}



kal_uint8 get_net_connect_state(void)
{
	xi_trace("haiming net_connect_ state:%d",chain.state);
    return  chain.state;
}

