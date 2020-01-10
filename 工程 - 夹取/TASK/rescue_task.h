#ifndef _rescue_task_H
#define _rescue_task_H


#include "stm32f4xx.h"

typedef struct
{
  uint8_t flag;
  uint8_t state;//记录是否初始化
  uint8_t last_state;
  
  uint8_t rescue_cmd;
  
  int16_t spd_ref[3];
  int16_t spd_fdb[3];
  
  int32_t angle_ref[3];
  int32_t angle_fdb[3];
  
  int32_t init_angle[3];//初始化角度
  
}rescue_t;

extern rescue_t rescue;

void rescue_task(void *parm);
void rescue_param_init(void);
void rescue_init_handler(void);
void rescue_enable_handler(void);


#endif
