#ifndef _upraise_task_H
#define _upraise_task_H


#include "stm32f4xx.h"

typedef enum
{
  DOWN = 0,
  UP,
  FALL,
  RAISE,
  UPRAISE_INIT,
}updown_flag_t;

typedef struct
{
  uint8_t flag;
  uint8_t state;
  
  updown_flag_t updown_flag;
  
  int16_t spd_ref[2];
  int16_t spd_fdb[2];
  
  int32_t angle_ref[2];
  int32_t angle_fdb[2];
  
  int32_t init_angle[2];
    
}upraise_t;

extern upraise_t upraise;

void upraise_task(void *parm);
void upraise_init_handler(void);
void upraise_param_init(void);

#endif

