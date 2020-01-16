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
    
    /*����ң������*/
//    send_rc_data1();
//    send_rc_data2();
//    send_rc_data3();
    
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
  last_global_mode = global_mode;//��ȡ��һ��ȫ��״̬
  last_gimbal_mode = gimbal_mode;//��ȡ��һ����̨״̬
  last_chassis_mode = chassis_mode;//��ȡ��һ�ε���״̬
  last_shoot_mode = shoot_mode;//��ȡ��һ�����״̬
}

void get_main_mode(void)
{   
  switch(rc.sw2)
  {
    case RC_UP:
    {
      if(TRACK_CTRL)//����Ҽ������Ӿ�
        global_mode = SEMI_AUTOMATIC_CTRL;
      else
        global_mode = MANUAL_CTRL;
    }
    break;
    case RC_MI:
    {
      global_mode = ENGINEER_CTRL;//��ȡ ��Ԯģʽ
      chassis.dodge_ctrl = 0;//�˳�����ģʽ�����Ƚ������ģʽ
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
        gimbal_mode = GIMBAL_NORMAL_MODE; //��̨���̸���ģʽ
    }
    break;
    
    case SEMI_AUTOMATIC_CTRL:
    {
       gimbal_mode = GIMBAL_TRACK_ARMOR; //��̨����ģʽ
    }
    break;
    
    case ENGINEER_CTRL:
    {
      if(RC_CHANGE_MODE)
      {
        gimbal.rescue_ctrl = !gimbal.rescue_ctrl;
        if(!gimbal.rescue_ctrl)
          gimbal_mode = GIMBAL_CLAMP_MODE; //��ȡģʽ
        else
          gimbal_mode = GIMBAL_SUPPLY_MODE;//����ģʽ
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
      if(RC_DODGE_MODE || chassis.dodge_ctrl) //����������ģʽ�����̽���С����
        chassis_mode = CHASSIS_DODGE_MODE;
      else                                    //������̨����ģʽ
        chassis_mode = CHASSIS_NORMAL_MODE;
    }break;
    
    case SEMI_AUTOMATIC_CTRL:  
    {
      if(chassis.dodge_ctrl)   //����������飬�ҿ������ģʽ�����̽���С����
        chassis_mode = CHASSIS_DODGE_MODE;
      else                          //������̨����ģʽ
        chassis_mode = CHASSIS_NORMAL_MODE;
    }break;
    
    case ENGINEER_CTRL:
    {
      if(gimbal_mode == GIMBAL_RESCUE_MODE)
        chassis_mode = CHASSIS_RESCUE_MODE; //���̾�Ԯģʽ
      
      if(gimbal_mode == GIMBAL_CLAMP_MODE)
        chassis_mode = CHASSIS_CLAMP_MODE; //���̼�ȡģʽ
      
      if(gimbal_mode == GIMBAL_SUPPLY_MODE)
        chassis_mode = CHASSIS_SUPPLY_MODE; //���̲���ģʽ
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
