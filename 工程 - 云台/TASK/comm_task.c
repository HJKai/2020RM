#include "comm_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "bsp_can.h"

UBaseType_t can_stack_surplus;

motor_current_t glb_cur;

void can_msg_send_task(void *parm)
{
	uint32_t Signal;
	BaseType_t STAUS;
  
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
										        (uint32_t) GIMBAL_MOTOR_MSG_SIGNAL  | \
                                       CHASSIS_MOTOR_MSG_SIGNAL | \
                                       SHOT_MOTOR_MSG_SIGNAL  | \
                                       MODE_SWITCH_MSG_SIGNAL, 
									        	(uint32_t *)&Signal, 
									        	(TickType_t) portMAX_DELAY );
		if(STAUS == pdTRUE)
		{
			if(Signal & GIMBAL_MOTOR_MSG_SIGNAL)//发送云台电流
			{
        send_gimbal_cur(glb_cur.gimbal_cur[0],glb_cur.gimbal_cur[1],glb_cur.gimbal_cur[2]);
      }
      if(Signal & CHASSIS_MOTOR_MSG_SIGNAL)//发送底盘电流
      {
        send_chassis_cur(glb_cur.chassis_cur[0],glb_cur.chassis_cur[1],glb_cur.chassis_cur[2],glb_cur.chassis_cur[3]);
      }
      if(Signal & SHOT_MOTOR_MSG_SIGNAL)//发送摩擦轮电流
      {
//        send_fric_cur(glb_cur.fric_cur[0],glb_cur.fric_cur[1]);
      }
      if(Signal & MODE_SWITCH_MSG_SIGNAL) //发送电流全为0
      {
        send_gimbal_cur(glb_cur.gimbal_cur[0],glb_cur.gimbal_cur[1],glb_cur.gimbal_cur[2]);
        send_chassis_cur(glb_cur.chassis_cur[0],glb_cur.chassis_cur[1],glb_cur.chassis_cur[2],glb_cur.chassis_cur[3]);
//        send_fric_cur(glb_cur.fric_cur[0],glb_cur.fric_cur[1]);
      }
		}
    
    can_stack_surplus = uxTaskGetStackHighWaterMark(NULL);

  }
}
