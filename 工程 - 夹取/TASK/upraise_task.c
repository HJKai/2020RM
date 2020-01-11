#include "upraise_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modeswitch_task.h"
#include "comm_task.h"
#include "clamp_task.h"
#include "pid.h"
#include "math.h"
#include "bsp_can.h"

extern TaskHandle_t can_msg_send_Task_Handle;
UBaseType_t upraise_stack_surplus;

upraise_t upraise;

uint32_t small_upraise_angle = 500,big_upraise_angle = 1000,clamp_upraise_angle = 500;
float upraise_pid[6] = {10.0f, 0, 0, 4.5f, 0, 0};

void upraise_task(void *parm)
{
	uint32_t Signal;
	BaseType_t STAUS;
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
										        (uint32_t) INFO_GET_SUPPLY_SIGNAL, 
									        	(uint32_t *)&Signal, 
									        	(TickType_t) portMAX_DELAY );
		if(STAUS == pdTRUE)
		{
			if(Signal & INFO_GET_SUPPLY_SIGNAL)
			{
        for(int i = 0; i < 2; i++)
        {
          PID_Struct_Init(&pid_upraise[i],upraise_pid[0],upraise_pid[1],upraise_pid[2],5000, 500, DONE);
          PID_Struct_Init(&pid_upraise_spd[i],upraise_pid[3],upraise_pid[4],upraise_pid[5],10000, 500, DONE);
        }
        if(upraise.state == INIT_DONE)
        {
          if(upraise.updown_flag == RAISE) //正在抬升
          {
            if(global_mode == GLOBAL_SUPPLY_MODE) //补给抬升
            {
              if(supply_mode == SUPPLY_TO_HERO)    //补大弹丸
              {
                upraise.angle_ref[0] = upraise.init_angle[0] + big_upraise_angle;
                upraise.angle_ref[1] = upraise.init_angle[1] - big_upraise_angle;
              }
              if(supply_mode == SUPPLY_TO_INFANTRY)  //补小弹丸
              {
                upraise.angle_ref[0] = upraise.init_angle[0] + small_upraise_angle;
                upraise.angle_ref[1] = upraise.init_angle[1] - small_upraise_angle;
              }
            }
            if(global_mode == GLOBAL_CLAMP_MODE) //夹取抬升
            {
              upraise.angle_ref[0] = upraise.init_angle[0] + clamp_upraise_angle;
              upraise.angle_ref[1] = upraise.init_angle[1] - clamp_upraise_angle;
            }
            if(fabs(upraise.angle_ref[0] - pid_upraise[0].get) < 10 && fabs(upraise.angle_ref[1] - pid_upraise[1].get) < 10)
            {
              upraise.updown_flag = UP;  //抬升完成
            }
            
          }
          
          if(clamp.clamp_flag == CLAMPED && upraise.updown_flag == FALL)  //正在下降    夹子收回来才能下降
          {
            upraise.angle_ref[0] = upraise.init_angle[0];
            upraise.angle_ref[1] = upraise.init_angle[1];
            if(fabs(pid_upraise[0].set - pid_upraise[0].get) < 10)
            {
              upraise.updown_flag = DOWN; //下降完成
            }
            
          }
        }
        else
        {
          upraise_init_handler();
        }
        
        for(int i = 0;i < 2;i++)
        {
          pid_calc(&pid_upraise[i], moto_upraise[i].total_angle, upraise.angle_ref[i]);
          if(upraise.state == INIT_NEVER)
            pid_calc(&pid_upraise_spd[i], upraise.spd_fdb[i], upraise.spd_ref[i]);
          else
            pid_calc(&pid_upraise_spd[i], upraise.spd_fdb[i], pid_upraise[i].out);
        }
        
        xTaskGenericNotify((TaskHandle_t) can_msg_send_Task_Handle, 
                           (uint32_t) UPRAISE_MOTOR_MSG_SIGNAL, 
                           (eNotifyAction) eSetBits, 
                           (uint32_t *)NULL );
        
        upraise_stack_surplus = uxTaskGetStackHighWaterMark(NULL);       
      }
    }

  }
}

void upraise_param_init(void)
{
  memset(&upraise, 0, sizeof(upraise_t));
  
  upraise.state = INIT_DONE;
  upraise.updown_flag = DOWN;

	for(int i = 0; i < 2; i++)
  {
    PID_Struct_Init(&pid_upraise[i],upraise_pid[0],upraise_pid[1],upraise_pid[2],5000, 500, INIT);
    PID_Struct_Init(&pid_upraise_spd[i],upraise_pid[3],upraise_pid[4],upraise_pid[5],10000, 500, INIT);
    upraise.init_angle[i] = moto_upraise[i].total_angle;
  }
    
}

void upraise_init_handler(void)
{
  static uint32_t error_angle[2] = {0};
  static uint8_t error_state[2] = {0};
  if(upraise.flag == 0)
  {
    /******下降初始化*****/
    upraise.spd_ref[0] = -500;//左边
    upraise.spd_ref[1] = 500;//右边
  }
  for(int i=0;i<2;i++)
  {
    if((fabs(pid_upraise_spd[i].set) - fabs(pid_upraise_spd[i].get)) > 0.7f*fabs(pid_upraise_spd[i].set))
    {
      error_angle[i]++;
      if(error_angle[i] > 250)
      {
        error_state[i] = 1;
        upraise.spd_ref[i] = 0;
        upraise.flag = 1;
      }
    }
  }
  if(error_state[0] == 1 && error_state[1] == 1)
  {
    for(int i=0;i<2;i++)
    {
      upraise.init_angle[i] = moto_upraise[i].total_angle;
      error_angle[i] = 0;
      error_state[i] = 0;
      upraise.state = INIT_DONE;
      upraise.flag = 0;
    }
  }
}

