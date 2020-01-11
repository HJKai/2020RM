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
  GPIO_ResetBits(GPIOC,GPIO_Pin_0); //补给气缸缩回来
  GPIO_SetBits(GPIOB,GPIO_Pin_1);
}

void supply_to_hero_handler(void)
{

  if(supply.supply_cmd)
  {
    GPIO_ResetBits(GPIOB,GPIO_Pin_1); //补给气缸伸出去
    GPIO_SetBits(GPIOC,GPIO_Pin_0);
  }
  else
  {
    GPIO_ResetBits(GPIOC,GPIO_Pin_0); //补给气缸缩回来
    GPIO_SetBits(GPIOB,GPIO_Pin_1);
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

