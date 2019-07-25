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
#include "stdint.h"
#include "stdbool.h"
#include "soc_api.h"

#define NULL (void *)0
/************************************************************************************
 * 
 * 
 * *********************************************************************************/

#define MEM_SIZE                    (32*1024)
#define SAMPLING_RATE               (1000)
#define NETWORK_RECONNECT_INTERVAL  (10*1000)


#define UVSENSOR_RAW_SIZE           (6)
#define MODULE_ID                   (10000)


 
/************************************************************************************
 * 
 * 
 * *********************************************************************************/
#define xi_trace(s...)         kal_prompt_trace(MOD_ENG,s)
#define xi_htons(x)            soc_htons(x)
#define xi_ntohs(x)            soc_htons(x)
#define xi_htonl(x)            soc_htonl(x)
#define xi_ntohl(x)            soc_ntohl(x)


extern time_t get_sys_timestamp();

extern int get_gsensor(kal_int16 *x,kal_int16 *y,kal_int16 *z);
extern kal_uint8 get_uvsensor_data(kal_uint8 *data);
extern void dump(uint8_t *data,int32_t size);



/************************************************************************************
 * 
 * 
 * *********************************************************************************/
extern void mem_init();
void *xi_malloc(uint32_t size);
void xi_free(void *mem);
