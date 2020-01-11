#include "modeswitch_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "remote_ctrl.h"
#include "keyboard.h"
#include "comm_task.h"
#include "gimbal_task.h"
#include "chassis_task.h"
#include "bsp_can.h"


UBaseType_t mode_switch_stack_surplus;

extern TaskHandle_t info_get_Task_Handle;

global_status global_mode; 
global_status last_global_mode;

gimbal_status gimbal_mode;
gimbal_status last_gimbal_mode;

chassis_status chassis_mode;
chassis_status last_chassis_mode;

shoot_status shoot_mode;
shoot_status last_shoot_mode;


void mode_switch_task(void *parm)
{
	uint32_t mode_switch_wake_time = osKernelSysTick();
  while(1)
  {
    get_last_mode();
    get_main_mode();
    get_gimbal_mode();
    get_chassis_mode();
    get_shoot_mode();
    
    /*发送遥控数据*/
    send_rc_data1();
    send_rc_data2();
    send_rc_data3();
    
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
  last_gimbal_mode = gimbal_mode;//获取上一次云台状态
  last_chassis_mode = chassis_mode;//获取上一次底盘状态
  last_shoot_mode = shoot_mode;//获取上一次射击状态
}

void get_main_mode(void)
{   
  switch(rc.sw2)
  {
    case RC_UP:
    {
      if(TRACK_CTRL)//鼠标右键进入视觉
        global_mode = SEMI_AUTOMATIC_CTRL;
      else
        global_mode = MANUAL_CTRL;
    }
    break;
    case RC_MI:
    {
      global_mode = ENGINEER_CTRL;//夹取 救援模式
      chassis.dodge_ctrl = 0;//退出工程模式后首先进入跟随模式
    }break;
    case RC_DN:
    {
      global_mode = RELEASE_CTRL;
      ramp_init(&pit_ramp, 1000);
      ramp_init(&yaw_ramp, 1000);
    }
    break;
    default:
    {
    }break;
  }
}

void get_gimbal_mode(void)
{  
  switch(global_mode)
  {
    case MANUAL_CTRL:
    { 
      if(chassis_mode == CHASSIS_DODGE_MODE)
        gimbal_mode = GIMBAL_DODGE_MODE; 
      else
        gimbal_mode = GIMBAL_NORMAL_MODE; //云台底盘跟随模式
    }
    break;
    
    case SEMI_AUTOMATIC_CTRL:
    {
       gimbal_mode = GIMBAL_TRACK_ARMOR; //云台自瞄模式
    }
    break;
    
    case ENGINEER_CTRL:
    {
      if(RC_CHANGE_MODE)
      {
        gimbal.rescue_ctrl = !gimbal.rescue_ctrl;
        if(!gimbal.rescue_ctrl)
          gimbal_mode = GIMBAL_CLAMP_MODE; //夹取模式
        else
          gimbal_mode = GIMBAL_SUPPLY_MODE;//补给模式
      }
      
      if(KB_CLAMP_MODE)
        gimbal_mode = GIMBAL_CLAMP_MODE;
      if(KB_SUPPLY_MODE)
        gimbal_mode = GIMBAL_SUPPLY_MODE;
      if(KB_RESCUE_MODE || RC_RESCUE_MODE)
        gimbal_mode = GIMBAL_RESCUE_MODE;
    }break;
    
    case RELEASE_CTRL:
    {
      gimbal_mode = GIMBAL_RELEASE;
      gimbal.state = GIMBAL_INIT_NEVER;
    }break;
    
  }
}

void get_chassis_mode(void)
{
  if(KB_DODGE_CTRL)
    chassis.dodge_ctrl = 1;
  if(KB_DODGE_CTRL_CLOSE)
    chassis.dodge_ctrl = 0;
  
  switch(global_mode)
  {
    case RELEASE_CTRL:
      chassis_mode = CHASSIS_RELEASE;
    break;
    
    case MANUAL_CTRL:
    {
      if(RC_DODGE_MODE || chassis.dodge_ctrl) //如果开启躲避模式，底盘进入小陀螺
        chassis_mode = CHASSIS_DODGE_MODE;
      else                                    //底盘云台跟随模式
        chassis_mode = CHASSIS_NORMAL_MODE;
    }break;
    
    case SEMI_AUTOMATIC_CTRL:  
    {
      if(chassis.dodge_ctrl)   //如果进入自瞄，且开启躲避模式，底盘进入小陀螺
        chassis_mode = CHASSIS_DODGE_MODE;
      else                          //底盘云台跟随模式
        chassis_mode = CHASSIS_NORMAL_MODE;
    }break;
    
    case ENGINEER_CTRL:
    {
      if(gimbal_mode == GIMBAL_RESCUE_MODE)
        chassis_mode = CHASSIS_RESCUE_MODE; //底盘救援模式
      
      if(gimbal_mode == GIMBAL_CLAMP_MODE)
        chassis_mode = CHASSIS_CLAMP_MODE; //底盘夹取模式
      
      if(gimbal_mode == GIMBAL_SUPPLY_MODE)
        chassis_mode = CHASSIS_SUPPLY_MODE; //底盘补给模式
    }break;
    
    default:
    {
    }break;
  }
}

void get_shoot_mode(void)
{
  switch(global_mode)
  {
    case MANUAL_CTRL:
    case SEMI_AUTOMATIC_CTRL:
    {
      shoot_mode = SHOOT_ENABLE;
    }break;
    case ENGINEER_CTRL:
    {
      shoot_mode = SHOOT_DISABLE;
    }break;
    
    case RELEASE_CTRL:
      shoot_mode = SHOOT_DISABLE;
    break;
    
    default:
    {
    }break;
  }
}
