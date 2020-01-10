#ifndef _remote_ctrl_H
#define _remote_ctrl_H

#include "stm32f4xx.h"
#include "rc.h"

/*救援控制*/
#define RC_RESCUE_MODE            ((glb_sw.last_sw1 == RC_MI) && (rc.sw1 == RC_UP))
/*夹取控制*/
#define RC_SMALL_SINGLE_CLAMP    	((glb_sw.last_sw1 == RC_MI) && (rc.sw1 == RC_DN))
#define RC_SMALL_CONTINUE_CLAMP  	(rc.sw1 == RC_DN) //夹取动作
#define RC_BIG_SINGLE_CLAMP    		((glb_sw.last_sw1 == RC_MI) && (rc.sw1 == RC_UP))
#define RC_BIG_CONTINUE_CLAMP  		(rc.sw1 == RC_UP) //夹取动作
/*补给控制*/
#define RC_SUPPLY_BIG	 				    (rc.ch4 >= 450)
#define RC_SUPPLY_SMALL	 	        (rc.ch4 <= -450)
#define RC_SUPPLY_CMD             (rc.sw1 == RC_DN) //补给动作
/*抬升初始化*/
#define RC_UPRAISE_INIT           ((rc.ch4 >= 450) && (rc.ch3 >= 450))

enum
{
  RC_UP = 1,
  RC_MI = 3,
  RC_DN = 2,
	IW_UP = 694,		//	min 364		middle 1024	拨轮
	IW_DN = 1354,		//	max 1684
};

typedef struct
{
	/*记录拨杆上一次状态*/
  uint8_t last_sw1;
  uint8_t last_sw2;
  uint16_t last_iw;
} sw_record_t;

typedef struct
{
  
} rc_ctrl_t;

extern rc_ctrl_t rm;
extern sw_record_t glb_sw;

void remote_ctrl_supply_hook(void);
void remote_ctrl_rescue_hook(void);
void remote_ctrl_clamp_hook(void);

#endif


