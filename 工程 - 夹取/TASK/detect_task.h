#ifndef _detect_task_H
#define _detect_task_H


#include "stm32f4xx.h"

#define Set_bit(data,bit) (data|=(1<<bit))
#define Reset_bit(data,bit) (data&=(~(1<<bit)))

typedef enum
{
  BOTTOM_DEVICE        = 0,
/*云台*/
  CHASSIS_M1_OFFLINE   = 1,
  CHASSIS_M2_OFFLINE   = 2,
  CHASSIS_M3_OFFLINE   = 3,
  CHASSIS_M4_OFFLINE   = 4,
  GIMBAL_YAW_OFFLINE   = 5,
  GIMBAL_PIT_OFFLINE   = 6,
  TRIGGER_MOTO_OFFLINE = 7,
  FRI_MOTO1_OFFLINE    = 8,
  FRI_MOTO2_OFFLINE    = 9,
  REMOTE_CTRL_OFFLINE1 = 10,
  JUDGE_SYS_OFFLINE1   = 11,
  PC_SYS_OFFLINE1      = 12,
/*夹取*/
  CLAMP_M1_OFFLINE     = 13,
  CLAMP_M2_OFFLINE     = 14,
  CLAMP_M3_OFFLINE     = 15,
  UPRAISE_M1_OFFLINE   = 16,
  UPRAISE_M2_OFFLINE   = 17,
  RESCUE_M1_OFFLINE    = 18,
  RESCUE_M2_OFFLINE    = 19,
  RESCUE_M3_OFFLINE    = 20,
  MCU_COMM_OFFLINE     = 21,
  JUDGE_SYS_OFFLINE    = 22,
  PC_SYS_OFFLINE       = 23,
  
  ERROR_LIST_LENGTH    = 24,
}err_id;

typedef union
{
	uint32_t offline;
	struct
	{
    uint32_t m0 : 1;
    uint32_t m1 : 1;
    uint32_t m2 : 1;
    uint32_t m3 : 1;
    uint32_t m4 : 1;
		uint32_t m5 : 1;
		uint32_t m6 : 1;
		uint32_t m7 : 1;
		uint32_t m8 : 1;
		uint32_t m9 : 1;
    uint32_t m10 : 1;
    uint32_t m11 : 1;
    uint32_t m12 : 1;
		uint32_t m13 : 1;
		uint32_t m14 : 1;
		uint32_t m15 : 1;
		uint32_t m16 : 1;
		uint32_t m17 : 1;
		uint32_t m18 : 1;
    uint32_t m19 : 1;
    uint32_t m20 : 1;
    uint32_t m21 : 1;
    uint32_t m22 : 1;
    uint32_t m23 : 1;
    uint32_t reserve : 8;
	}bit;
}State_t;

typedef struct
{
  uint32_t last_times;
  uint32_t delta_times;
  uint16_t set_timeout;
  
}detect_param_t;

typedef struct
{
  uint8_t err_exist;
  detect_param_t param; 
  
}err_status;

typedef struct
{
  uint8_t err_now_id[ERROR_LIST_LENGTH];
  err_status list[ERROR_LIST_LENGTH];
  uint32_t offline;
  uint32_t gimbal_offline; //接收云台主控板的离线数据
}global_err_t;

extern global_err_t global_err;
extern State_t state;

void detect_task(void *parm);
void detect_param_init(void);
void err_detector_hook(int err_id);
void module_offline_detect(void);
void module_offline_callback(void);

uint8_t clamp_is_controllable(void);
uint8_t rescue_is_controllable(void);
uint8_t supply_is_controllable(void);

#endif

