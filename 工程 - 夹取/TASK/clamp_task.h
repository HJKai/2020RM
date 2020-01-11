#ifndef _clamp_task_H
#define _clamp_task_H


#include "stm32f4xx.h"

typedef enum
{
  CLAMPING = 0,
  CLAMPED,
}clamp_flag_t;

typedef enum
{
  INIT_BOX = 0, //�м�λ��
  FRIST_BOX,
  SECOND_BOX,
  THRID_BOX,
  FOURTH_BOX,
}pick_box_t;

typedef struct
{
  int16_t offset_angle_left;
  int16_t offset_angle_mid;
  int16_t offset_angle_right;
}offset_angle_t;

typedef struct
{
  offset_angle_t small_island;
  offset_angle_t big_island;
}island_offset;

typedef struct
{
  uint8_t init_flag;
  uint8_t state;
  uint8_t last_state;
  
  clamp_flag_t clamp_flag;
  uint8_t island_ctrl;
  
  uint8_t      clamp_cmd;
  uint8_t      c_clamp_cmd;
  
  int16_t spd_ref[3];
  int16_t spd_fdb[3];
  
  int32_t angle_ref[3];
  int32_t angle_fdb[3];
  
  int32_t init_angle[3];//��ʼ���Ƕ�
  
  uint32_t  c_clamp_time; //�ж�ң��   		
  
}clamp_t;

extern clamp_t clamp;
extern pick_box_t pick_box;

void clamp_task(void *parm);

void clamp_param_init(void);
void clamp_init_handler(void);
void small_island_handler(void);
void big_island_handler(void);
void auto_small_island_handler(void);
void auto_big_island_handler(void);

#endif


