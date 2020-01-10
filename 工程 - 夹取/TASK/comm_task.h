#ifndef _comm_task_H
#define _comm_task_H


#include "stm32f4xx.h"

#define JUDGE_UART_TX_SIGNAL    ( 1 << 0 )
#define JUDGE_UART_IDLE_SIGNAL  ( 1 << 1 )

#define PC_UART_TX_SIGNAL       ( 1 << 2 )
#define PC_UART_IDLE_SIGNAL     ( 1 << 3 )

#define CLAMP_MOTOR_MSG_SIGNAL  ( 1 << 4 )
#define RESCUE_MOTOR_MSG_SIGNAL ( 1 << 5 )
#define UPRAISE_MOTOR_MSG_SIGNAL ( 1 << 6 )
#define MODE_SWITCH_MSG_SIGNAL  ( 1 << 7 )

#define INFO_GET_CLAMP_SIGNAL   ( 1 << 8 )
#define INFO_GET_RESCUE_SIGNAL  ( 1 << 9 )
#define INFO_GET_SUPPLY_SIGNAL  ( 1 << 10 )
#define INFO_GET_UPRAISE_SIGNAL  ( 1 << 11 )

#define MODE_SWITCH_INFO_SIGNAL ( 1 << 12 )


typedef struct
{
  /* 底盘电机电流 */
  int16_t chassis_cur[4];
  /* yaw/pitch/trigger 电机电流 */
  int16_t gimbal_cur[3];
  /*摩擦轮电机电流*/
  int16_t fric_cur[2];
} motor_current_t;

extern motor_current_t glb_cur;

void can_msg_send_task(void *parm);


#endif 

