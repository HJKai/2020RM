#include "info_get_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "comm_task.h"
#include "keyboard.h"
#include "remote_ctrl.h"
#include "bsp_can.h"
#include "chassis_task.h"
#include "gimbal_task.h"
#include "shoot_task.h"
#include "modeswitch_task.h"
#include "imu_task.h"
#include "sys_config.h"
#include "pc_rx_data.h"
#include "stdlib.h"
#include "math.h"

UBaseType_t info_stack_surplus;

extern TaskHandle_t can_msg_send_Task_Handle;
extern TaskHandle_t gimbal_Task_Handle;
extern TaskHandle_t chassis_Task_Handle;
extern TaskHandle_t shoot_Task_Handle;


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
        
        keyboard_global_hook();
        get_chassis_info();
        get_gimbal_info();
        get_shoot_info();
        get_global_last_info();
        
        taskEXIT_CRITICAL();
        if(global_mode == RELEASE_CTRL)
        {
          MEMSET(&glb_cur,motor_current_t);
          xTaskGenericNotify( (TaskHandle_t) can_msg_send_Task_Handle, 
                              (uint32_t) MODE_SWITCH_MSG_SIGNAL, 
                              (eNotifyAction) eSetBits, 
                              (uint32_t *)NULL );
        } 
        else
        {
          xTaskGenericNotify( (TaskHandle_t) gimbal_Task_Handle, 
                            (uint32_t) INFO_GET_GIMBAL_SIGNAL, 
                            (eNotifyAction) eSetBits, 
                            (uint32_t *)NULL );
          xTaskGenericNotify( (TaskHandle_t) chassis_Task_Handle, 
                            (uint32_t) INFO_GET_CHASSIS_SIGNAL, 
                            (eNotifyAction) eSetBits, 
                            (uint32_t *)NULL );
          xTaskGenericNotify( (TaskHandle_t) shoot_Task_Handle, 
                            (uint32_t) INFO_GET_SHOOT_SIGNAL, 
                            (eNotifyAction) eSetBits, 
                            (uint32_t *)NULL );
        }
        
        info_stack_surplus = uxTaskGetStackHighWaterMark(NULL);       
      }
    }

  }
}

static void get_chassis_info(void)
{
  /* 得到底盘轮速 */
  for (uint8_t i = 0; i < 4; i++)
  {
    chassis.wheel_spd_fdb[i] = moto_chassis[i].speed_rpm;
  }
  /* get remote and keyboard chassis control information */
  keyboard_chassis_hook();
  remote_ctrl_chassis_hook();
  
  /* get chassis structure configuration parameter */
  get_structure_param();
	
}

static void get_structure_param(void)
{
  if ((pc_recv_mesg.structure_data.chassis_config == CUSTOM_CONFIG)
   && (pc_recv_mesg.structure_data.wheel_perimeter != 0)
   && (pc_recv_mesg.structure_data.wheel_base      != 0)
   && (pc_recv_mesg.structure_data.wheel_track     != 0))
  {
    glb_struct.chassis_config  = CUSTOM_CONFIG;
    glb_struct.wheel_perimeter = pc_recv_mesg.structure_data.wheel_perimeter;
    glb_struct.wheel_base      = pc_recv_mesg.structure_data.wheel_base;
    glb_struct.wheel_track     = pc_recv_mesg.structure_data.wheel_track;
  }
  else
  {
    glb_struct.chassis_config  = DEFAULT_CONFIG;
    glb_struct.wheel_perimeter = PERIMETER;
    glb_struct.wheel_base      = WHEELBASE;
    glb_struct.wheel_track     = WHEELTRACK;
  }

  if ((pc_recv_mesg.structure_data.gimbal_config == CUSTOM_CONFIG)
   && (abs(pc_recv_mesg.structure_data.gimbal_x_offset) < pc_recv_mesg.structure_data.wheel_base/2)
   && (abs(pc_recv_mesg.structure_data.gimbal_y_offset) < pc_recv_mesg.structure_data.wheel_track/2))
  {
    glb_struct.gimbal_config   = CUSTOM_CONFIG;
    glb_struct.gimbal_x_offset = pc_recv_mesg.structure_data.gimbal_x_offset;
    glb_struct.gimbal_y_offset = pc_recv_mesg.structure_data.gimbal_y_offset;
  }
  else
  {
    glb_struct.gimbal_config   = DEFAULT_CONFIG;
    glb_struct.gimbal_x_offset = GIMBAL_X_OFFSET;
    glb_struct.gimbal_y_offset = GIMBAL_Y_OFFSET;
  }
}

int yaw_spd = 1;
int pit_spd = 1;

static void get_gimbal_info(void)
{
  /* 转换成相对角度 */
  static float yaw_ecd_ratio = YAW_MOTO_POSITIVE_DIR*YAW_DECELE_RATIO/ENCODER_ANGLE_RATIO;
  static float pit_ecd_ratio = PIT_MOTO_POSITIVE_DIR*PIT_DECELE_RATIO/ENCODER_ANGLE_RATIO;
  gimbal.sensor.yaw_relative_angle = yaw_ecd_ratio*get_relative_pos(moto_yaw.ecd, gimbal.yaw_center_offset);
  gimbal.sensor.pit_relative_angle = pit_ecd_ratio*get_relative_pos(moto_pit.ecd, gimbal.pit_center_offset);

  /*获取遥控和键鼠的数据*/
  keyboard_gimbal_hook();
  remote_ctrl_gimbal_hook();
  
}

static int16_t get_relative_pos(int16_t raw_ecd, int16_t center_offset)
{
  int16_t tmp = 0;
  if (center_offset >= 4096)						//分中轴线象限
  {
		if (raw_ecd - center_offset > - 4096)
      tmp = raw_ecd - center_offset;							//在右边 = 负数
    else
      tmp = raw_ecd + 8192 - center_offset;				//在左边 = 正数
  }
  else
  {
		if (raw_ecd - center_offset > 4096)
      tmp = raw_ecd - 8192 - center_offset;				//在右边 = 负数
    else
      tmp = raw_ecd - center_offset;							//在左边 = 正数
  }
  return tmp;
}

static void get_shoot_info(void)
{
  remote_ctrl_shoot_hook();
  keyboard_shoot_hook();
}

static void get_global_last_info(void)
{
  glb_sw.last_sw1 = rc.sw1;
  glb_sw.last_sw2 = rc.sw2;  
  glb_sw.last_iw = rc.iw; 
}
