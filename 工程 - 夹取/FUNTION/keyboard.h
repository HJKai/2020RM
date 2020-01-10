#ifndef _keyboard_H
#define _keyboard_H


#include "stm32f4xx.h"

//      rescue ctrl
#define KB_RESCUE_CTRL (rc.kb.bit.E) //救援
#define KB_CLOSE_RESCUE_CTRL (rc.kb.bit.SHIFT & rc.kb.bit.E) //关闭救援
//      clamp ctrl
#define VISUAL_CTRL (km.r_mouse_sta == MOUSE_LONG) //识别夹取
#define KB_SINGLE_CLAMP     (km.l_mouse_sta == MOUSE_ONCE)
#define KB_CONTINUE_CLAMP   (km.l_mouse_sta == MOUSE_LONG) //夹取动作
#define KB_CTRL_CLAMP_SMALL (rc.kb.bit.CTRL & rc.kb.bit.Q) //小资源岛
#define KB_CTRL_CLAMP_BIG   (rc.kb.bit.SHIFT && rc.kb.bit.Q) //大资源岛
//      supply ctrl
#define KB_SUPPLY_SMALL		(rc.kb.bit.CTRL && rc.kb.bit.F) //补给小弹丸
#define KB_SUPPLY_BIG		  (rc.kb.bit.SHIFT & rc.kb.bit.F) //补给大弹丸
#define KB_SUPPLY         (km.l_mouse_sta == MOUSE_LONG) //补给动作 
//      upraise init
#define KB_UPRAISE_INIT   (rc.kb.bit.C)//初始化抬升电机
/**********************************************************************************
 * bit      :15   14   13   12   11   10   9   8   7   6     5     4   3   2   1
 * keyboard : V    C    X	  Z    G    F    R   E   Q  CTRL  SHIFT  D   A   S   W
 **********************************************************************************/
//#define W 			0x0001		//bit 0
//#define S 			0x0002
//#define A 			0x0004
//#define D 			0x0008
//#define SHIFT 	0x0010
//#define CTRL 		0x0020
//#define Q 			0x0040
//#define E				0x0080
//#define R 			0x0100
//#define F 			0x0200
//#define G 			0x0400
//#define Z 			0x0800
//#define X 			0x1000
//#define C 			0x2000
//#define V 			0x4000		//bit 15
//#define B				0x8000
/******************************************************/

typedef enum 
{
  MOUSE_RELEASE,
  MOUSE_PRESS,
  MOUSE_DONE,
  MOUSE_ONCE,
  MOUSE_LONG,
  
}MOUSE_STATUS;

typedef struct
{
  MOUSE_STATUS l_mouse_sta;
  MOUSE_STATUS r_mouse_sta;
  uint16_t l_cnt;
  uint16_t r_cnt;
  
  uint8_t kb_enable;
  
}kb_ctrl_t;

extern kb_ctrl_t km;

void keyboard_global_hook(void);
void keyboard_clamp_hook(void);
void keyboard_rescue_hook(void);
void keyboard_supply_hook(void);


#endif
