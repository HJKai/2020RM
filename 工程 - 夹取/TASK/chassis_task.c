#include "chassis_task.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modeswitch_task.h"
#include "comm_task.h"
#include "gimbal_task.h"
#include "info_get_task.h"
#include "detect_task.h"
#include "pid.h"
#include "sys_config.h"
#include "stdlib.h"
#include "math.h"
#include "pc_rx_data.h"
#include "remote_ctrl.h"
#include "keyboard.h"

UBaseType_t chassis_stack_surplus;
extern TaskHandle_t can_msg_send_Task_Handle;

chassis_t chassis;

float chassis_pid[6] = {13.0f, 0.0f, 90.0f, 4.5f, 0.05, 0};

void chassis_task(void *parm)
{
  uint32_t Signal;
	BaseType_t STAUS;
  
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
										        (uint32_t) INFO_GET_CHASSIS_SIGNAL, 
									        	(uint32_t *)&Signal, 
									        	(TickType_t) portMAX_DELAY );
    if(STAUS == pdTRUE)
		{
			if(Signal & INFO_GET_CHASSIS_SIGNAL)
			{
        /*����vw��ת��pid*/
        PID_Struct_Init(&pid_chassis_angle,chassis_pid[0],chassis_pid[1],chassis_pid[2],MAX_CHASSIS_VR_SPEED, 50, DONE);
        /*����vx,vyƽ�Ƶ�pid*/
        for(int i = 0; i < 4; i++)
        {
          PID_Struct_Init(&pid_spd[i],chassis_pid[3],chassis_pid[4],chassis_pid[5],10000, 500, DONE);
        }
        
        if(chassis_mode != CHASSIS_RELEASE && gimbal.state != GIMBAL_INIT_NEVER)//��̨����֮����̲��ܶ�
        {
          switch(chassis_mode)
          {
            /*��̨���̸���ģʽ*/
            case CHASSIS_NORMAL_MODE:
            {
              chassis_normal_handler();
            }break;
            /*��̨���̷���ģʽ*/
            case CHASSIS_SEPARATE_MODE:
            {
              chassis_separate_handler();
            }break;
            /*С����ģʽ*/
            case CHASSIS_DODGE_MODE:
            {
              chassis_dodge_handler();
            }break;
            /*����ֹͣģʽ*/
            case CHASSIS_STOP_MODE:
            {
              chassis_stop_handler();
            }break;
            
            default:
            {
            }break;
          }
          
          mecanum_calc(chassis.vx, chassis.vy, chassis.vw, chassis.wheel_spd_ref);

          for (int i = 0; i < 4; i++)
          {
            chassis.current[i] = pid_calc(&pid_spd[i], chassis.wheel_spd_fdb[i], chassis.wheel_spd_ref[i]);
          }
          
          if (!chassis_is_controllable())
          {
            memset(chassis.current, 0, sizeof(chassis.current));
          }
          
          memcpy(glb_cur.chassis_cur, chassis.current, sizeof(chassis.current));
        }
        else
        {
          memset(glb_cur.chassis_cur, 0, sizeof(glb_cur.chassis_cur));
        }
        
        xTaskGenericNotify( (TaskHandle_t) can_msg_send_Task_Handle, 
                  (uint32_t) CHASSIS_MOTOR_MSG_SIGNAL, 
                  (eNotifyAction) eSetBits, 
                  (uint32_t *)NULL );
      }
    }
    
    chassis_stack_surplus = uxTaskGetStackHighWaterMark(NULL);
  }
}


static void chassis_normal_handler(void)
{
  int position_ref = 0;
	
	chassis.vy = (rm.vy * CHASSIS_RC_MOVE_RATIO_Y + km.vy * CHASSIS_KB_MOVE_RATIO_Y);
  chassis.vx = (rm.vx * CHASSIS_RC_MOVE_RATIO_X + km.vx * CHASSIS_KB_MOVE_RATIO_X);
//  if(input_flag == 0)
    chassis.vw = (-pid_calc(&pid_chassis_angle, gimbal.sensor.yaw_relative_angle, position_ref))*0.9f;// ������λ chassis_ratio;
//  else
//    chassis.vw = 0;
}

static void chassis_separate_handler(void)
{	
  chassis.vy = rm.vy * CHASSIS_RC_MOVE_RATIO_Y + km.vy * CHASSIS_KB_MOVE_RATIO_Y;
  chassis.vx = rm.vx * CHASSIS_RC_MOVE_RATIO_X + km.vx * CHASSIS_KB_MOVE_RATIO_X;
  chassis.vw = rm.vw * CHASSIS_RC_MOVE_RATIO_R;
}

static void chassis_dodge_handler(void)
{
  float cha_x, cha_y , dodge_angle;
	/*С����*/
  dodge_angle = gimbal.sensor.yaw_relative_angle;
	cha_x = (rm.vx * CHASSIS_RC_MOVE_RATIO_X + km.vx * CHASSIS_KB_MOVE_RATIO_X);
	cha_y = (rm.vy * CHASSIS_RC_MOVE_RATIO_Y + km.vy * CHASSIS_KB_MOVE_RATIO_Y);
	chassis.vy = (cha_y * cos(-PI/180*dodge_angle) - cha_x * 0.5f * sin(-PI/180*dodge_angle));
	chassis.vx = (cha_x * cos(-PI/180*dodge_angle) + cha_y * 0.5f * sin(-PI/180*dodge_angle));
	chassis.vw = LG_SPEED;							//300
}

static void chassis_stop_handler(void)
{
  chassis.vy = 0;
  chassis.vx = 0;
  chassis.vw = 0;
}

void chassis_param_init(void)
{
  memset(&chassis,0,sizeof(chassis));
  
  /*����vw��ת��pid*/
  PID_Struct_Init(&pid_chassis_angle,chassis_pid[0],chassis_pid[1],chassis_pid[2],MAX_CHASSIS_VR_SPEED, 50, INIT);
  /*����vx,vyƽ�Ƶ�pid*/
  for(int i = 0; i < 4; i++)
    PID_Struct_Init(&pid_spd[i],chassis_pid[3],chassis_pid[4],chassis_pid[5],10000, 500, INIT);
  
}



/**
  * @brief mecanum chassis velocity decomposition
  * @param input : ��=+vx(mm/s)  ��=+vy(mm/s)  ccw=+vw(deg/s)
  *        output: every wheel speed(rpm)
	* @trans ���룺		ǰ�����ҵ���
	*				 �����		ÿ�����Ӷ�Ӧ���ٶ�
  * @note  1=FR 2=FL 3=BL 4=BR
	* @work	 �������㹫ʽ�����Ч��
  */
int rotation_center_gimbal = 0;
static void mecanum_calc(float vx, float vy, float vw, int16_t speed[])
{
  static float rotate_ratio_fr;
  static float rotate_ratio_fl;
  static float rotate_ratio_bl;
  static float rotate_ratio_br;
  static float wheel_rpm_ratio;
  
  taskENTER_CRITICAL();
  if(chassis_mode == CHASSIS_DODGE_MODE || chassis.dodge_ctrl)
  {
    chassis.rotate_x_offset = GIMBAL_X_OFFSET;
    chassis.rotate_y_offset = 0;
  }
  else
  {
    if (rotation_center_gimbal)
    {
      chassis.rotate_x_offset = glb_struct.gimbal_x_offset;
      chassis.rotate_y_offset = glb_struct.gimbal_y_offset;
    }
    else
    {
      chassis.rotate_x_offset = 0;
      chassis.rotate_y_offset = 0;
    }
  }
  //@work
  rotate_ratio_fr = ((glb_struct.wheel_base+glb_struct.wheel_track)/2.0f \
                      - chassis.rotate_x_offset + chassis.rotate_y_offset)/RADIAN_COEF;
  rotate_ratio_fl = ((glb_struct.wheel_base+glb_struct.wheel_track)/2.0f \
                      - chassis.rotate_x_offset - chassis.rotate_y_offset)/RADIAN_COEF;
  rotate_ratio_bl = ((glb_struct.wheel_base+glb_struct.wheel_track)/2.0f \
                      + chassis.rotate_x_offset - chassis.rotate_y_offset)/RADIAN_COEF;
  rotate_ratio_br = ((glb_struct.wheel_base+glb_struct.wheel_track)/2.0f \
                      + chassis.rotate_x_offset + chassis.rotate_y_offset)/RADIAN_COEF;

  wheel_rpm_ratio = 60.0f/(glb_struct.wheel_perimeter*CHASSIS_DECELE_RATIO);
  taskEXIT_CRITICAL();
  

  VAL_LIMIT(vx, -MAX_CHASSIS_VX_SPEED, MAX_CHASSIS_VX_SPEED);  //mm/s
  VAL_LIMIT(vy, -MAX_CHASSIS_VY_SPEED, MAX_CHASSIS_VY_SPEED);  //mm/s
	/*С���������ģʽ��vw��������*/
	if(chassis_mode != CHASSIS_DODGE_MODE && !chassis.dodge_ctrl)
    VAL_LIMIT(vw, -MAX_CHASSIS_VR_SPEED, MAX_CHASSIS_VR_SPEED);  //deg/s
	/*С����ʱ��vw��������*/
	else
		VAL_LIMIT(vw, -MAX_CHASSIS_VR_SPEED, MAX_CHASSIS_VR_SPEED);  //deg/s

  int16_t wheel_rpm[4];
  float   max = 0;
  
  wheel_rpm[0] = (-vx - vy - vw * rotate_ratio_fr) * wheel_rpm_ratio;
  wheel_rpm[1] = ( vx - vy - vw * rotate_ratio_fl) * wheel_rpm_ratio;
  wheel_rpm[2] = ( vx + vy - vw * rotate_ratio_bl) * wheel_rpm_ratio;
  wheel_rpm[3] = (-vx + vy - vw * rotate_ratio_br) * wheel_rpm_ratio;

  //find max item
  for (uint8_t i = 0; i < 4; i++)
  {
    if (abs(wheel_rpm[i]) > max)
      max = abs(wheel_rpm[i]);
  }
  //equal proportion
  if (max > MAX_WHEEL_RPM)
  {
    float rate = MAX_WHEEL_RPM / max;
    for (uint8_t i = 0; i < 4; i++)
      wheel_rpm[i] *= rate;
  }
  memcpy(speed, wheel_rpm, 4*sizeof(int16_t));
}
