#ifndef _sys_config_H
#define _sys_config_H

#include "stm32f4xx.h"

#define DEBUG

#define INFANTRY_1  1

#define  INFANTRY_NUM  INFANTRY_1

/*3508的减速比 */
#define DECELE_RATIO_3508 (19.0f/1.0f)
/*2006的减速比*/
#define DECELE_RATIO_2006 (36.0f/1.0f)

/* 电机编码值 和 角度（度） 的比率 */
#define ENCODER_ANGLE_RATIO    (8192.0f/360.0f)


/*********************** 系统 交互接口 配置 ****************************/

/* CAN 相关 */
#define CLAMP_CAN           CAN1
#define RESCUE_CAN          CAN1

#define UPRAISE_CAN         CAN2


/* 计算 相关 */
/* 弧度 系数 */
#define RADIAN_COEF        57.3f
/* 圆周率 */
#define PI                 3.142f

//判断最大最小值
#define VAL_LIMIT(val, min, max) \
do {\
if((val) <= (min))\
{\
  (val) = (min);\
}\
else if((val) >= (max))\
{\
  (val) = (max);\
}\
} while(0)\

#endif


