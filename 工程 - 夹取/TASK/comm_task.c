#include "comm_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "bsp_can.h"
#include "pid.h"

UBaseType_t can_stack_surplus;

motor_current_t glb_cur;

void can_msg_send_task(void *parm)
{
	uint32_t Signal;
	BaseType_t STAUS;
  
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
										        (uint32_t) CLAMP_MOTOR_MSG_SIGNAL    | \
                                       RESCUE_MOTOR_MSG_SIGNAL   | \
                                       UPRAISE_MOTOR_MSG_SIGNAL,
									        	(uint32_t *)&Signal, 
									        	(TickType_t) portMAX_DELAY );
		if(STAUS == pdTRUE)
		{
			if(Signal & CLAMP_MOTOR_MSG_SIGNAL)//发送夹取电机电流
			{
        send_clamp_cur(pid_clamp_spd[0].out,pid_clamp_spd[1].out,pid_clamp_spd[2].out);
      }
      if(Signal & RESCUE_MOTOR_MSG_SIGNAL)//发送救援电机电流
      {
//        send_rescue_cur(pid_rescue_spd[0].out,pid_rescue_spd[1].out,pid_rescue_spd[2].out);
      }
      if(Signal & UPRAISE_MOTOR_MSG_SIGNAL)//发送抬升电机电流
      {
        send_upraise_cur(pid_upraise_spd[0].out,pid_upraise_spd[1].out);
      }
		}
    
    can_stack_surplus = uxTaskGetStackHighWaterMark(NULL);

  }
}
