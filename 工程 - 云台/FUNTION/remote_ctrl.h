#ifndef _remote_ctrl_H
#define _remote_ctrl_H

#include "stm32f4xx.h"
#include "rc.h"

/*µ×ÅÌ¿ØÖÆ*/
#define RC_DODGE_MODE             ((rc.sw1 == RC_MI) && (rc.sw2 == RC_UP) && (rc.iw >= IW_DN))

/*Éä»÷¿ØÖÆ*/
#define RC_SINGLE_SHOOT    				((glb_sw.last_sw1 == RC_MI) && (rc.sw1 == RC_DN))
#define RC_CONTINUE_SHOOT  				(rc.sw1 == RC_DN)
#define RC_CTRL_FRIC_WHEEL 				((glb_sw.last_sw1 == RC_MI) && (rc.sw1 == RC_UP))
#define RC_CTRL_BALL_STOR	 				((rc.sw2 == RC_UP) && (glb_sw.last_iw > IW_UP) && (rc.iw <= IW_UP))

/*¾ÈÔ®*/
#define RC_CHANGE_MODE            ((rc.sw2 == RC_MI) && (glb_sw.last_iw < IW_DN) && (rc.iw >= IW_DN))
#define RC_RESCUE_MODE            ((rc.sw2 == RC_MI) && (glb_sw.last_iw > IW_UP) && (rc.iw <= IW_UP))

enum
{
  RC_UP = 1,
  RC_MI = 3,
  RC_DN = 2,
	IW_UP = 694,		//	min 364		middle 1024	²¦ÂÖ
	IW_DN = 1354,		//	max 1684
};

typedef struct
{
	/*¼ÇÂ¼²¦¸ËºÍ²¦ÂÖÉÏÒ»´Î×´Ì¬*/
  uint8_t last_sw1;
  uint8_t last_sw2;
  uint16_t last_iw;
} sw_record_t;

typedef struct
{
	/*µ×ÅÌ·½Ïò*/
  float vx;
  float vy;
  float vw;
  /*ÔÆÌ¨·½Ïò*/
  float pit_v;
  float yaw_v;
} rc_ctrl_t;

extern rc_ctrl_t rm;
extern sw_record_t glb_sw;

void remote_ctrl(rc_info_t *rc,uint8_t *dbus_buf);
void remote_ctrl_chassis_hook(void);
void remote_ctrl_gimbal_hook(void);
void remote_ctrl_shoot_hook(void);

#endif


