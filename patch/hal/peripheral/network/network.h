#ifndef _NETWORK_H_
#define _NETWORK_H_


#define INVALID_SOCKET          (-1)
#define URL 	                {47,101,213,152}
#define PORT	                (18888)
#define BUF_SIZE                (1024)

typedef struct {
    uint8_t  ip[4];
    uint16_t port;
    uint16_t rsv;
}socket_attr_t;

typedef struct {
    int8_t      socket_id;
    bool        state;
    uint32_t    frame_size;
    uint32_t    frame_sended_size;
    uint8_t     send_buf[BUF_SIZE];
    uint8_t     recv_buf[BUF_SIZE];
    uint8_t     module_id[MODULE_ID_LEN+1];
}chain_t;


extern void network_get_module_id(uint8_t *module_id);
extern void network_update_ip_port(uint8_t *ip,uint8_t size);



#endif
