#ifndef _shoot_task_H
#define _shoot_task_H


#include "stm32f4xx.h"

typedef enum
{
  SHOT_DISABLE       = 0,
  REMOTE_CTRL_SHOT   = 1,
  KEYBOARD_CTRL_SHOT = 2,
  SEMIAUTO_CTRL_SHOT = 3,
  AUTO_CTRL_SHOT     = 4,
} shoot_mode_e;



typedef __packed struct
{
  /* shoot task relevant param */
  shoot_mode_e ctrl_mode;
	shoot_mode_e last_ctrl_mode;
  uint8_t      shoot_cmd;
  uint32_t     c_shoot_time;   		//continuous
  uint8_t      c_shoot_cmd;
	
  uint8_t			 ball_storage_open;	//ball storage
	
  uint8_t      fric_wheel_run; 		//run or not
  uint16_t     fric_wheel_spd;
	
} shoot_t;

typedef __packed struct
{
  /* trigger motor param */
  int32_t   angle_ref;
  int32_t   spd_ref;
  uint32_t  one_time;
  int32_t   shoot_spd;   //单发
  int32_t   c_shoot_spd;			 //连发
  
} trigger_t;


extern shoot_t   shoot;
extern trigger_t trig;

void shoot_task(void *parm);

void shoot_param_init(void);
static void shoot_bullet_handler(void);
static void shoot_speed_ctrl(void);
static void fric_wheel_ctrl(void);
static void turn_on_friction_wheel(int16_t speed);
static void turn_off_friction_wheel(void);
static void ball_storage_ctrl(void);


#endif

