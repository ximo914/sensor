#include "port.h"
#include "protocol.h"
#include "sampling.h"

sampling_body_t sampling_body;

void sampling_packet(sampling_item_t *item)
{
    uint16_t gsensor[3];
    uint16_t uvsensor[6];
   uint16_t  i;

    get_gsensor((kal_int16 *)&gsensor[0],(kal_int16 *)&gsensor[1],(kal_int16 *)&gsensor[2]);
    xi_trace("gsensor--x:%x,y:%x,z:%x",gsensor[0],gsensor[1],gsensor[2]);
    item->timestamp = xi_htonl(get_sys_timestamp());
    item->gsensor_x = xi_htons(gsensor[0]);
    item->gsensor_y = xi_htons(gsensor[1]);
    item->gsensor_z = xi_htons(gsensor[2]);
     
    si1133_get_uv_data((kal_uint16 *)uvsensor);
    xi_trace("usensor--%x,%x,%x,%x,%x,%x",uvsensor[0],uvsensor[1],uvsensor[2],uvsensor[3],uvsensor[4],uvsensor[5]);
    for (i = 0; i < UVSENSOR_RAW_SIZE;i++) {
        item->uvsensor[i] = xi_htons(uvsensor[i]);
    }
}


void sampling()
{
    uint8_t i = 0;
    static uint32_t j = 0;

    xi_trace("sampling...");

    sampling_packet(&sampling_body.item[j]);
    j++;
    xi_trace("j:%d",j);
    if (j >= SAMPLE_CNT) {
        j = 0;
        protocol_send_data(OPT_CODE_SAMPLING,(uint8_t *)&sampling_body,sizeof(sampling_body_t));
    
    }    
    StopTimer(TIMER_ID_SAMPING);
    StartTimer(TIMER_ID_SAMPING,SAMPLING_RATE,sampling);
}

void sampling_upload_response(uint32_t timestamp,uint32_t seq,uint8_t *data,uint32_t size)
{

}

void sampling_opt_code_upload(uint32_t timestamp,uint32_t seq,uint8_t *data,uint32_t size)
{
    sampling();
}

void sampling_config(uint32_t timestamp,uint32_t seq,uint8_t *data,uint32_t size)
{
    ip_config_t *config = (ip_config_t *)data;
    xi_trace("ip[0]:%d,ip[1]:%d,ip[2]:%d,ip[3]:%d",config->ip[0],config->ip[1],config->ip[2],config->ip[3]);
    network_update_ip_port(&config->ip[0],4);
}


opt_code_table_t op_table[] = {
	{OPT_CODE_SAMPLING,sampling_upload_response},
    {OPT_CODE_CMD_CONFIG,sampling_config},
};

void sampling_frame(uint16_t opt_code,uint8_t packet_type,uint32_t timestamp,uint32_t seq,uint8_t *data,uint32_t size)
{
    uint32_t table_size = sizeof(op_table)/sizeof(op_table[0]);
    uint32_t i;

    xi_trace("recv server sampling....");
    xi_trace("code:%d,packet_type:%d,timestamp:%d,seq:%d",opt_code,packet_type,timestamp,seq);
    dump(data,size);
#if 1
    for(i = 0; i < table_size;i++) {
        if (opt_code == op_table[i].opt_code) {
            op_table[i].function(timestamp,seq,data,size);
            break;
        }
    }
#endif
}

void upload_low_voltage()
{
    xi_trace("upload_low_voltage....");
    protocol_send_data(OPT_CODE_UPLOAD_LOW_VOLATE,NULL,0);
}

