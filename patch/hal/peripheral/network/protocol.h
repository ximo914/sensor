#ifndef _PROTOCOL_XI_H_
#define _PROTOCOL_XI_H_

#define REQUEST         (0)
#define RESPOND         (1)

#define CRC_FIX_SIZE    (27)
#define MODULE_ID_LEN   (15)

#define SAMPLE_CNT (240)


#pragma pack(1)

typedef struct {
    uint16_t size;
    uint16_t size_complete;
    uint32_t crc32;

    uint32_t seq;
    uint8_t  protocol_version;
    uint8_t  packet_type;
    uint8_t  module_id[MODULE_ID_LEN];
    uint32_t timestamp;
    uint16_t op_code;
    uint8_t  buf[1];
}protocol_head_t;

typedef struct {
    uint32_t    timestamp;
    uint16_t    gsensor_x;
    uint16_t    gsensor_y;
    uint16_t    gsensor_z;
    uint16_t    uvsensor[6]; 
}sampling_item_t;

typedef struct {
    sampling_item_t item[SAMPLE_CNT]
}sampling_body_t;

typedef struct {
    uint8_t  ip[4];
}ip_config_t;

typedef enum{
    OPT_CODE_SAMPLING = 0,              /*主动上报*/
    OPT_CODE_CMD_CONFIG = 1,
    OPT_CODE_UPLOAD_LOW_VOLATE = 2,
}opt_code_enum;

#pragma pack()

extern int32_t network_send_frame(uint8_t *data,uint32_t size);
extern int32_t protocol_parse(uint8_t *data,uint32_t size);
extern int32_t protocol_send_data(uint16_t op_code,uint8_t *data,uint16_t size);


#endif