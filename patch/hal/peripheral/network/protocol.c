#include "port.h"
#include "protocol.h"
#include "sampling.h"
#include "crc32.h"

uint32_t protocol_get_seq()
{
    static uint32_t seq = 0;

    return seq++;
}

protocol_head_t* protocol_packet(uint16_t op_code,uint8_t packet_type,uint8_t *data,uint16_t size)
{
    protocol_head_t *packet = xi_malloc(sizeof(protocol_head_t) + size - 1);
    if (packet == NULL) {
        xi_trace("malloc failed...");
        return NULL;
    }
    packet->size = sizeof(protocol_head_t) + size - sizeof(uint16_t) - 1;
    packet->size_complete = xi_htons(0xFFFF - packet->size);
    packet->size = xi_htons(packet->size);

    packet->seq = xi_htonl(protocol_get_seq());
    packet->protocol_version = 1;
    packet->packet_type = packet_type;
    network_get_module_id(&packet->module_id[0]);
    packet->timestamp = xi_htonl(get_sys_timestamp());
    packet->op_code = xi_htons(op_code);
    if (size > 0) {
        memcpy(packet->buf,data,size);
    }
#if 0
    xi_trace("protocol packet size:%x",packet->size);
    xi_trace("protocol packet size_complete:%x",packet->size_complete);
    xi_trace("protocol packet module_id:%x",packet->module_id);
    xi_trace("protocol packet timestamp:%x",packet->timestamp);
#endif
    xi_trace("protocol packet timestamp:%d",get_sys_timestamp());

    packet->crc32 = xi_htonl(~crc32_update_bytes(~0L, (uint8_t*)&packet->seq, size + CRC_FIX_SIZE));
    return packet;
}

int32_t protocol_send_data(uint16_t op_code,uint8_t *data,uint16_t size)
{
    protocol_head_t *packet;

    packet = protocol_packet(op_code,REQUEST,data,size);
    network_send_frame((uint8_t *)packet,sizeof(protocol_head_t) + size - 1);
    xi_free(packet);
    packet = NULL;
    return true;
}

int32_t protocol_parse(uint8_t *data,uint32_t size)
{
    protocol_head_t *packet = (protocol_head_t *)data;
    uint32_t crc32;

    if (packet->size + packet->size_complete != 0xffff) {
        return false;
    }

    crc32 = ~crc32_update_bytes(~0L, (uint8_t*)&packet->seq, size - 8);

    xi_trace("calc crc32:%x,recv calc:%x",crc32,xi_ntohl(packet->crc32));
    //TODO 
#if 1
    if (xi_ntohl(packet->crc32) != crc32) {
        xi_trace("verify error");
        return false;;
    }
#endif
    sampling_frame(xi_ntohs(packet->op_code),packet->packet_type,xi_ntohl(packet->timestamp),xi_ntohl(packet->seq),packet->buf,size - sizeof(protocol_head_t) - 1);
    return true;
}