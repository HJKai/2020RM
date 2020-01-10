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

typedef enum
{
  TRIG_INIT       = 0,
  TRIG_PRESS_DOWN = 1,
  TRIG_BOUNCE_UP  = 2,
  TRIG_ONE_DONE   = 3,
} trig_state_e;

typedef enum
{
	SHOOTNOR_MODE			= 1,
	SHOOTMAD_MODE			= 2,
	SHOOTTHROW_MODE		= 3,
	SHOOTBUFF_MODE		= 4,
}shoot_para_e;


typedef __packed struct
{
  /* shoot task relevant param */
  shoot_mode_e ctrl_mode;
	shoot_mode_e last_ctrl_mode;
  uint8_t      shoot_cmd;
  uint32_t     c_shoot_time;   		//continuous
  uint8_t      c_shoot_cmd;
	
	uint8_t      last_para_mode;
	uint8_t			 para_mode;
  uint8_t			 ball_storage_open;	//ball storage
	
  uint8_t      fric_wheel_run; 		//run or not
  uint16_t     fric_wheel_spd;
	uint8_t			 fric_wheel_compe;	//摩擦轮转速补偿
	uint16_t		 fric_launch_time;
	
} shoot_t;

typedef __packed struct
{
  /* trigger motor param */
  int32_t   angle_ref;
  int32_t   spd_ref;
  uint32_t  one_time;
  int32_t   shoot_spd;   //单发
  int32_t   c_shoot_spd;			 //连发
  trig_state_e one_sta;
  
} trigger_t;

typedef enum
{
  SHOOT_CMD,
  FRIC_CTRL,
} shoot_type_e;

extern shoot_t   shoot;
extern trigger_t trig;

void shoot_task(void *parm);

void shoot_param_init(void);
void get_last_shoot_mode(void);
static void shoot_bullet_handler(void);
static void shoot_para_ctrl(void);
static void fric_wheel_ctrl(void);
static void turn_on_friction_wheel(int16_t lspd,int16_t rspd);
static void turn_off_friction_wheel(void);
static void ball_storage_ctrl(void);


#endif

