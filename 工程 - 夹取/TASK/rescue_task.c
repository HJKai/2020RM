#include "rescue_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modeswitch_task.h"
#include "comm_task.h"
#include "pid.h"
#include "bsp_can.h"
#include "math.h"

extern TaskHandle_t rescue_Task_Handle;
UBaseType_t rescue_stack_surplus;

rescue_t rescue;

float grasp_motoget = 90;//给定角度
float flex_motoget = 360;//
float rescue_pid[6] = {13.0f, 0.0f, 90.0f, 4.5f, 0.05, 0};

void rescue_task(void *parm)
{
	uint32_t Signal;
	BaseType_t STAUS;
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
										        (uint32_t) INFO_GET_RESCUE_SIGNAL, 
									        	(uint32_t *)&Signal, 
									        	(TickType_t) portMAX_DELAY );
		if(STAUS == pdTRUE)
		{
			if(Signal & INFO_GET_RESCUE_SIGNAL)
			{
        for(int i = 0; i < 3; i++)
        {
          PID_Struct_Init(&pid_rescue[i],rescue_pid[0],rescue_pid[1],rescue_pid[2],5000, 500, DONE);
          PID_Struct_Init(&pid_rescue_spd[i],rescue_pid[3],rescue_pid[4],rescue_pid[5],10000, 500, DONE);
        }
        
        if(global_mode == GLOBAL_RESCUE_MODE)
        {
          if(rescue.state == INIT_NEVER)
          {
             rescue_mode = RESCUE_INIT;
          }
          
          switch(rescue_mode)
          {
            case RESCUE_INIT:
            {
              rescue_init_handler();
            }break;
            
            case RESCUE_ENABLE:
            {
              rescue_enable_handler();
            }break;
            
            default:
            {
              
            }break;
            
          }
          
        }
        else
        {
          for(int i=0;i<3;i++)
          {
            rescue.angle_ref[i] = moto_rescue[i].total_angle;
            rescue.spd_ref[i] = rescue.spd_fdb[i];
          }
        }
        for(int i = 0;i < 3;i++)
        {
          pid_calc(&pid_rescue[i], moto_rescue[i].total_angle, rescue.angle_ref[i]);
          if(rescue.state == INIT_NEVER)
            pid_calc(&pid_rescue_spd[i], rescue.spd_fdb[i], rescue.spd_ref[i]);
          else
            pid_calc(&pid_rescue_spd[i], rescue.spd_fdb[i], pid_rescue[i].out);
        }
        
        xTaskGenericNotify((TaskHandle_t) rescue_Task_Handle, 
                           (uint32_t) RESCUE_MOTOR_MSG_SIGNAL, 
                           (eNotifyAction) eSetBits, 
                           (uint32_t *)NULL );
        
        rescue_stack_surplus = uxTaskGetStackHighWaterMark(NULL);       
      }
    }

  }
}


void rescue_param_init(void)
{
  memset(&rescue, 0, sizeof(rescue_t));
  
  rescue.flag = 0;
  rescue.state = INIT_DONE;
  rescue.last_state = INIT_NEVER;

	for(int i = 0; i < 3; i++)
  {
    PID_Struct_Init(&pid_rescue[i],rescue_pid[0],rescue_pid[1],rescue_pid[2],5000, 500, INIT);
    PID_Struct_Init(&pid_rescue_spd[i],rescue_pid[3],rescue_pid[4],rescue_pid[5],10000, 500, INIT);
  }
  
}


void rescue_init_handler(void)
{
  static uint32_t error_angle[3] = {0};
  static uint8_t error_state[3] = {0};
  if((rescue.flag == 0) || (rescue.state != rescue.last_state))
  {
    rescue.spd_ref[0] = 500;//左边
    rescue.spd_ref[1] = -500;//右边
    rescue.spd_ref[2] = 500;//下方
  }
  for(int i=0;i<3;i++)
  {
    if((fabs(pid_rescue_spd[i].set) - fabs(pid_rescue_spd[i].get)) > 0.7f*fabs(pid_rescue_spd[i].set))
    {
      error_angle[i]++;
      if(error_angle[i] > 250)
      {
        error_state[i] = 1;
        rescue.spd_ref[i] = 0;
        rescue.flag = 1;
      }
    }
  }
  if(error_state[0] == 1 && error_state[1] == 1 && error_state[2] == 1)
  {
    for(int i=0;i<3;i++)
    {
      rescue.init_angle[i] = moto_rescue[i].total_angle;
      error_angle[i] = 0;
      error_state[i] = 0;
      rescue.state = INIT_DONE;
      rescue.flag = 0;
//      rescue_mode = RESCUE_ENABLE;
    }
  }
}

void rescue_enable_handler(void)
{
  if(rescue.rescue_cmd)
  {
    rescue.angle_ref[0] = rescue.init_angle[0] + grasp_motoget;
    rescue.angle_ref[1] = rescue.init_angle[1] - grasp_motoget;
    rescue.angle_ref[2] = rescue.init_angle[2] + flex_motoget;
  }
  else
  {
    rescue.angle_ref[0] = rescue.init_angle[0];
    rescue.angle_ref[1] = rescue.init_angle[1];
    rescue.angle_ref[2] = rescue.init_angle[2];
  }
  
}

