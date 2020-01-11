#include "modeswitch_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "remote_ctrl.h"
#include "keyboard.h"
#include "comm_task.h"
#include "clamp_task.h"
#include "rescue_task.h"
#include "supply_task.h"
#include "upraise_task.h"
#include "ramp.h"

UBaseType_t mode_switch_stack_surplus;

extern TaskHandle_t info_get_Task_Handle;

global_status global_mode; 
global_status last_global_mode;

clamp_status clamp_mode;
clamp_status last_clamp_mode;

rescue_status rescue_mode;
rescue_status last_rescue_mode;

supply_status supply_mode;
supply_status last_supply_mode;

ramp_t rescue_ramp;

void mode_switch_task(void *parm)
{
	uint32_t mode_switch_wake_time = osKernelSysTick();
  while(1)
  {
    keyboard_global_hook();
    get_main_mode();
    get_last_mode();
    xTaskGenericNotify( (TaskHandle_t) info_get_Task_Handle, 
                        (uint32_t) MODE_SWITCH_INFO_SIGNAL, 
                        (eNotifyAction) eSetBits, 
                        (uint32_t *)NULL );
    
    mode_switch_stack_surplus = uxTaskGetStackHighWaterMark(NULL);
    
    vTaskDelayUntil(&mode_switch_wake_time, 2);
  }
}

void get_last_mode(void)
{
  last_global_mode = global_mode;//获取上一次全局状态
  last_clamp_mode = clamp_mode;//获取上一次夹取状态
  last_rescue_mode = rescue_mode;//获取上一次救援状态
  last_supply_mode = supply_mode;//获取上一次补给状态
}

void get_main_mode(void)
{   
  switch(global_mode)
  {
    case GLOBAL_CLAMP_MODE:
    {
      get_clamp_mode();
    }break;
    
    case GLOBAL_RESCUE_MODE:
    {
      get_rescue_mode();
    }break;
    
    case GLOBAL_SUPPLY_MODE:
    {
      get_supply_mode();
    }break;
    
    default:
    {
      upraise.updown_flag = FALL; //下降标志
    }break;
  }
}

void get_clamp_mode(void)
{  
  remote_ctrl_clamp_hook();
  keyboard_clamp_hook();

  if(VISUAL_CTRL)
  {
    if(clamp.island_ctrl == BIG_ISLAND)
      clamp_mode = AUTOMATIC_BIG_ISLAND;
    else
      clamp_mode = AUTOMATIC_SMALL_ISLAND;
  }
  else
  {
      if(clamp.island_ctrl == BIG_ISLAND)
        clamp_mode = BIG_ISLAND;
      else
        clamp_mode = SMALL_ISLAND;
  }
  
  if(last_global_mode != GLOBAL_CLAMP_MODE)
    upraise.updown_flag = RAISE; //抬升标志
}

void get_rescue_mode(void)
{ 
  remote_ctrl_rescue_hook();
  keyboard_rescue_hook();
  upraise.updown_flag = FALL; //下降标志
}

void get_supply_mode(void)
{
  remote_ctrl_supply_hook();
  keyboard_supply_hook();
  
  if(last_global_mode != GLOBAL_SUPPLY_MODE)
    upraise.updown_flag = RAISE; //抬升标志
}
