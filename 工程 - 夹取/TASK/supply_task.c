#include "supply_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modeswitch_task.h"
#include "comm_task.h"
#include "pid.h"
#include "math.h"
#include "bsp_can.h"

extern TaskHandle_t can_msg_send_Task_Handle;
UBaseType_t supply_stack_surplus;

supply_t supply;

void supply_task(void *parm)
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
        
        if(global_mode == GLOBAL_SUPPLY_MODE)
        {          
          switch(supply_mode)
          {
            
            case SUPPLY_TO_HERO:
            {
              supply_to_hero_handler();
            }break;
            
            case SUPPLY_TO_INFANTRY:
            {
              supply_to_infantry_handler();
            }break;
            
            default:
            {
              
            }break;
            
          }
          
        }
        else
        {
          
        }
        
        
        supply_stack_surplus = uxTaskGetStackHighWaterMark(NULL);       
      }
    }

  }
}


void supply_param_init(void)
{
//  memset(&upraise, 0, sizeof(upraise_t));
//  
//  upraise.state = INIT_NEVER;

//	for(int i = 0; i < 2; i++)
//  {
//    PID_Struct_Init(&pid_upraise[i],upraise_pid[0],upraise_pid[1],upraise_pid[2],5000, 500, INIT);
//    PID_Struct_Init(&pid_upraise_spd[i],upraise_pid[3],upraise_pid[4],upraise_pid[5],10000, 500, INIT);
//  }
  
}



void supply_to_hero_handler(void)
{

  if(supply.supply_cmd)
  {
    
  }
  else
  {
    
  }
}

void supply_to_infantry_handler(void)
{
  if(supply.supply_cmd)
  {
    TIM5->CCR4 = 1500;
  }
  else
  { 
    TIM5->CCR4 = 500;
  }
}

