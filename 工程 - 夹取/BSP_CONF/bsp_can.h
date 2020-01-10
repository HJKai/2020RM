#ifndef _bsp_can_H
#define _bsp_can_H

#include "stm32f4xx.h"


/* CAN 发送和接收 ID */
typedef enum
{
  CAN_CLAMP_M1_ID       = 0x201, //3508
  CAN_CLAMP_M2_ID       = 0x202,
  CAN_CLAMP_M3_ID       = 0x203,
  
  CAN_RESCUE_M1_ID      = 0x204, //2006
  CAN_RESCUE_M2_ID	    = 0x205,
  CAN_RESCUE_M3_ID 	    = 0x206,
 
  CAN_CLAMP_ALL_ID      = 0x200, 
  CAN_RESCUE_ALL_ID     = 0x1FF, 
} can1_msg_id_e;

typedef enum
{
  CAN_UPRAISE_M1_ID     = 0x201, //3508
  CAN_UPRAISE_M2_ID     = 0x202,
  
  CAN_UPRAISE_ALL_ID    = 0x200, 
} can2_msg_id_e;

typedef struct
{
  uint16_t ecd;
  uint16_t last_ecd;
  
  int16_t  speed_rpm;
  int16_t  given_current;

  int32_t  round_cnt;
  int32_t  total_ecd;
  int32_t  total_angle;
  
  uint16_t offset_ecd;
  uint32_t msg_cnt;
  
  int32_t  ecd_raw_rate;
} moto_measure_t;


extern moto_measure_t moto_clamp[3]; //夹取移动   3508
extern moto_measure_t moto_upraise[2]; //抬升机构 3508
extern moto_measure_t moto_rescue[3]; //救援机构  2006

void encoder_data_handler(moto_measure_t* ptr, CanRxMsg *message);
void get_moto_offset(moto_measure_t* ptr, CanRxMsg *message);

void send_clamp_cur(int16_t iq1, int16_t iq2, int16_t iq3);
void send_upraise_cur(int16_t iq1, int16_t iq2);
void send_rescue_cur(int16_t iq1, int16_t iq2, int16_t iq3);

#endif 

