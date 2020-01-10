#ifndef _chassis_task_H
#define _chassis_task_H


#include "stm32f4xx.h"

typedef struct
{
  uint8_t         dodge_ctrl;
  float           vx; // forward/back
  float           vy; // left/right
  float           vw; // 
  int16_t         rotate_x_offset;
  int16_t         rotate_y_offset;
  
  int16_t         wheel_spd_fdb[4];
  int16_t         wheel_spd_ref[4];
  int16_t         current[4];
  
}chassis_t;

extern chassis_t chassis;

void chassis_task(void *parm);
void chassis_param_init(void);

static void chassis_normal_handler(void);
static void chassis_separate_handler(void);
static void chassis_dodge_handler(void);
static void chassis_stop_handler(void);
static void mecanum_calc(float vx, float vy, float vw, int16_t speed[]);

#endif
