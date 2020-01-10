#include "info_get_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "comm_task.h"
#include "keyboard.h"
#include "remote_ctrl.h"
#include "bsp_can.h"
#include "modeswitch_task.h"
#include "clamp_task.h"
#include "rescue_task.h"
#include "supply_task.h"
#include "upraise_task.h"
#include "sys_config.h"
#include "pc_rx_data.h"
#include "stdlib.h"
#include "math.h"

UBaseType_t info_stack_surplus;

extern TaskHandle_t clamp_Task_Handle;
extern TaskHandle_t rescue_Task_Handle;
extern TaskHandle_t supply_Task_Handle;
extern TaskHandle_t upraise_Task_Handle;

void info_get_task(void *parm)
{
	uint32_t Signal;
	BaseType_t STAUS;
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
										        (uint32_t) MODE_SWITCH_INFO_SIGNAL, 
									        	(uint32_t *)&Signal, 
									        	(TickType_t) portMAX_DELAY );
		if(STAUS == pdTRUE)
		{
			if(Signal & MODE_SWITCH_INFO_SIGNAL)
			{
        taskENTER_CRITICAL();
        
        get_clamp_info(); //移动夹取电机的数据
        get_rescue_info(); //救援电机的数据
        get_upraise_info(); //抬升电机的数据
        get_global_last_info();
        
        taskEXIT_CRITICAL();

        xTaskGenericNotify( (TaskHandle_t) clamp_Task_Handle, 
                          (uint32_t) INFO_GET_CLAMP_SIGNAL, 
                          (eNotifyAction) eSetBits, 
                          (uint32_t *)NULL );
        xTaskGenericNotify( (TaskHandle_t) rescue_Task_Handle, 
                          (uint32_t) INFO_GET_RESCUE_SIGNAL, 
                          (eNotifyAction) eSetBits, 
                          (uint32_t *)NULL );
        xTaskGenericNotify( (TaskHandle_t) supply_Task_Handle, 
                          (uint32_t) INFO_GET_SUPPLY_SIGNAL, 
                          (eNotifyAction) eSetBits, 
                          (uint32_t *)NULL );
        xTaskGenericNotify( (TaskHandle_t) upraise_Task_Handle, 
                          (uint32_t) INFO_GET_SUPPLY_SIGNAL, 
                          (eNotifyAction) eSetBits, 
                          (uint32_t *)NULL );                          
        
        info_stack_surplus = uxTaskGetStackHighWaterMark(NULL);       
      }
    }

  }
}

static void get_clamp_info(void)
{
  /* 获取夹取电机角度和转速 */
  for (uint8_t i = 0; i < 3; i++)
  {
    clamp.spd_fdb[i] = moto_clamp[i].speed_rpm;
    clamp.angle_fdb[i] = moto_clamp[i].total_angle;
  }
	
}

static void get_rescue_info(void)
{
  /* 获取救援电机角度和转速 */
  for (uint8_t i = 0; i < 3; i++)
  {
    rescue.spd_fdb[i] = moto_rescue[i].speed_rpm;
    rescue.angle_fdb[i] = moto_rescue[i].total_angle;
  }
  
}

static void get_upraise_info(void)
{
  /* 获取抬升电机角度和转速 */
  for (uint8_t i = 0; i < 2; i++)
  {
    upraise.spd_fdb[i] = moto_upraise[i].speed_rpm;
    upraise.angle_fdb[i] = moto_upraise[i].total_angle;
  }
  
}

static void get_global_last_info(void)
{
  glb_sw.last_sw1 = rc.sw1;
  glb_sw.last_sw2 = rc.sw2;  
}
