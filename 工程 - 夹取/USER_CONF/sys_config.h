#ifndef _sys_config_H
#define _sys_config_H

#include "stm32f4xx.h"

#define DEBUG

#define INFANTRY_1  1

#define  INFANTRY_NUM  INFANTRY_1

/*3508�ļ��ٱ� */
#define DECELE_RATIO_3508 (19.0f/1.0f)
/*2006�ļ��ٱ�*/
#define DECELE_RATIO_2006 (36.0f/1.0f)

/* �������ֵ �� �Ƕȣ��ȣ� �ı��� */
#define ENCODER_ANGLE_RATIO    (8192.0f/360.0f)


/*********************** ϵͳ �����ӿ� ���� ****************************/

/* CAN ��� */
#define CLAMP_CAN           CAN1
#define RESCUE_CAN          CAN1

#define UPRAISE_CAN         CAN2


/* ���� ��� */
/* ���� ϵ�� */
#define RADIAN_COEF        57.3f
/* Բ���� */
#define PI                 3.142f

//�ж������Сֵ
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


