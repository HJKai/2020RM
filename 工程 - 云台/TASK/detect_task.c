#include "detect_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modeswitch_task.h"
#include "bsp_can.h"

RCC_ClocksTypeDef RCC_Clocks;

UBaseType_t detect_stack_surplus;

global_err_t global_err;

void detect_task(void *parm)
{
	uint32_t detect_wake_time = osKernelSysTick();
  while(1)
  {
    module_offline_detect();
//    module_offline_callback();
    
    send_detect_state();
    RCC_GetClocksFreq(&RCC_Clocks);//检测外部晶振是否起振
    
    detect_stack_surplus = uxTaskGetStackHighWaterMark(NULL);    
    vTaskDelayUntil(&detect_wake_time, 50);
  }
}


void detect_param_init(void)
{
  for(uint8_t id = CHASSIS_M1_OFFLINE; id <= JUDGE_SYS_OFFLINE; id++)
  {
    global_err.list[id].param.set_timeout = 500;
    global_err.list[id].param.last_times = 0;
    global_err.list[id].param.delta_times = 0;
    global_err.list[id].err_exist = 0;
    global_err.err_now_id[id] = BOTTOM_DEVICE;
  }
  global_err.list[PC_SYS_OFFLINE].param.set_timeout = 2000;
  global_err.list[PC_SYS_OFFLINE].param.last_times = 0;
  global_err.list[PC_SYS_OFFLINE].param.delta_times = 0;
  global_err.list[PC_SYS_OFFLINE].err_exist = 0;
  global_err.err_now_id[PC_SYS_OFFLINE] = BOTTOM_DEVICE;
}

void err_detector_hook(int err_id)
{
  global_err.list[err_id].param.last_times = HAL_GetTick();
}

void module_offline_detect(void)
{
  for (uint8_t id = CHASSIS_M1_OFFLINE; id <= PC_SYS_OFFLINE; id++)
  {
    global_err.list[id].param.delta_times = HAL_GetTick() - global_err.list[id].param.last_times;
    if(global_err.list[id].param.delta_times > global_err.list[id].param.set_timeout)
    {
      global_err.err_now_id[id] = (err_id)id;
      global_err.list[id].err_exist = 1;
      Set_bit(global_err.offline,id);
    }
    else
    {
      global_err.err_now_id[id] = BOTTOM_DEVICE;
      global_err.list[id].err_exist = 0;
      Reset_bit(global_err.offline,id);
    }
  }
}

void module_offline_callback(void)
{
  for(uint8_t id = CHASSIS_M1_OFFLINE; id <= PC_SYS_OFFLINE; id++)
  {
    switch(global_err.err_now_id[id])
    {
      case CHASSIS_M1_OFFLINE:
      {
//        Set_bit(global_err.offline,CHASSIS_M1_OFFLINE);
      }break;
      case CHASSIS_M2_OFFLINE:
      {
//        Set_bit(global_err.offline,CHASSIS_M2_OFFLINE);
      }break;       
      case CHASSIS_M3_OFFLINE:
      {
//        Set_bit(global_err.offline,CHASSIS_M3_OFFLINE);
      }break;
      case CHASSIS_M4_OFFLINE:
      {
//        Set_bit(global_err.offline,CHASSIS_M4_OFFLINE);
      }break;        
      case GIMBAL_YAW_OFFLINE:
      {
//        Set_bit(global_err.offline,GIMBAL_YAW_OFFLINE);
      }break;        
      case GIMBAL_PIT_OFFLINE:
      {
//        Set_bit(global_err.offline,GIMBAL_PIT_OFFLINE);
      }break;       
      case TRIGGER_MOTO_OFFLINE:
      {
//        Set_bit(global_err.offline,TRIGGER_MOTO_OFFLINE);
      }break;       
      case FRI_MOTO1_OFFLINE:
      {
//        Set_bit(global_err.offline,FRI_MOTO1_OFFLINE);
      }break;       
      case FRI_MOTO2_OFFLINE:
      {
//        Set_bit(global_err.offline,FRI_MOTO2_OFFLINE);
      }break;       
      case REMOTE_CTRL_OFFLINE:
      {
//        Set_bit(global_err.offline,REMOTE_CTRL_OFFLINE);
      }break;        
      case JUDGE_SYS_OFFLINE:
      {
//        Set_bit(global_err.offline,JUDGE_SYS_OFFLINE);
      }break;        
      case PC_SYS_OFFLINE:
      {
//        Set_bit(global_err.offline,PC_SYS_OFFLINE);
      }break;
      default:
      {
        /*用户处理代码*/
      }break;
    }
  }
}

uint8_t gimbal_is_controllable(void)
{
  if (gimbal_mode == GIMBAL_RELEASE
   || global_err.list[REMOTE_CTRL_OFFLINE].err_exist
   || global_err.list[GIMBAL_YAW_OFFLINE].err_exist
   || global_err.list[GIMBAL_PIT_OFFLINE].err_exist)
    return 0;
  else
    return 1;
}

uint8_t chassis_is_controllable(void)
{
  if (chassis_mode == CHASSIS_RELEASE 
   || global_err.list[REMOTE_CTRL_OFFLINE].err_exist)
    return 0;
  else
    return 1;
}

uint8_t shoot_is_controllable(void)
{
  if (shoot_mode == SHOOT_DISABLE 
   || global_err.list[REMOTE_CTRL_OFFLINE].err_exist)
    return 0;
  else
    return 1;
}
