#ifndef _SAMPLING_H_
#define _SAMPLING_H_

typedef struct {
    uint16_t opt_code;
    void (*function)(uint32_t timestamp,uint32_t seq,uint8_t *data,uint32_t size);
}opt_code_table_t;

extern void sampling_frame(uint16_t opt_code,uint8_t packet_type,uint32_t timestamp,uint32_t seq,uint8_t *data,uint32_t size);
extern void sampling();
extern void upload_low_voltage();


#endif