#include "gimbal_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modeswitch_task.h"
#include "comm_task.h"
#include "detect_task.h"
#include "bsp_can.h"
#include "pid.h"
#include "remote_ctrl.h"
#include "keyboard.h"
#include "sys_config.h"
#include "pc_rx_data.h"
#include "pid.h"
#include "stdlib.h"
#include "stdlib.h" //abs()函数
#include "math.h"   //fabs()函数

UBaseType_t gimbal_stack_surplus;
extern TaskHandle_t can_msg_send_Task_Handle;

ramp_t pit_ramp;
ramp_t yaw_ramp;

#if (INFANTRY_NUM == INFANTRY_1)
float pit_pid[6] = {30,0,0,9,0,0};
float yaw_pid[6] = {30, 0, 0, 80, 0, 0};
float trig_pid[6] = {250,0,120,10,0,0};

float pit_dir = 1.f;
float yaw_dir = 1.f;

float pit_ctrl_ffc;
float yaw_ctrl_ffc;

FFC p_ffc;
FFC y_ffc;

#define PIT_ANGLE_MAX      15
#define PIT_ANGLE_MIN      -25
#define YAW_ANGLE_MAX      50
#define YAW_ANGLE_MIN      -50

#endif

gimbal_t gimbal;
uint32_t gimbal_time,last_gimbal_time;
void gimbal_task(void *parm)
{
  uint32_t Signal;
	BaseType_t STAUS;
  
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
										        (uint32_t) INFO_GET_GIMBAL_SIGNAL, 
									        	(uint32_t *)&Signal, 
									        	(TickType_t) portMAX_DELAY );
    if(STAUS == pdTRUE)
		{
			if(Signal & INFO_GET_GIMBAL_SIGNAL)
			{
        gimbal_time = HAL_GetTick() - last_gimbal_time;
        last_gimbal_time = HAL_GetTick();
        
        /* pit 轴电机的PID参数 */
        PID_Struct_Init(&pid_pit, pit_pid[0], pit_pid[1], pit_pid[2], 3000, 500, DONE);
        PID_Struct_Init(&pid_pit_spd, pit_pid[3], pit_pid[4], pit_pid[5], 6000, 4000, DONE);

        /* yaw 轴电机的PID参数 */
        PID_Struct_Init(&pid_yaw, yaw_pid[0], yaw_pid[1], yaw_pid[2], 5000, 500, DONE); 
        PID_Struct_Init(&pid_yaw_spd, yaw_pid[3] , yaw_pid[4], yaw_pid[5], 30000, 15000, DONE);  
  
        /* 拨盘 电机的PID参数 */
        PID_Struct_Init(&pid_trigger, trig_pid[0], trig_pid[1], trig_pid[2], 8000, 0, DONE);
        PID_Struct_Init(&pid_trigger_spd, trig_pid[3], trig_pid[4], trig_pid[5],8000, 3000,DONE);
        
        if(gimbal_mode != GIMBAL_RELEASE)
        {
          if(gimbal.state == GIMBAL_INIT_NEVER)
          {
            gimbal_mode = GIMBAL_INIT;
          }
          switch(gimbal_mode)
          {
            case GIMBAL_INIT:
            {
              init_mode_handler(); //云台回中
            }break;
            /*云台底盘跟随模式*/
            case GIMBAL_NORMAL_MODE:
            {
              nomarl_handler();
            }break;
            /*云台底盘分离模式*/
            case GIMBAL_SEPARATE_MODE:
            {
              separate_handler();
            }break;
            /*小陀螺模式*/
            case GIMBAL_DODGE_MODE:
            {
              dodge_handler();
            }break;
            /*打能量机关模式*/
            case GIMBAL_SHOOT_BUFF:
            {
              shoot_buff_ctrl_handler();
            }break;
            /*自瞄模式*/
            case GIMBAL_TRACK_ARMOR:
            {
              track_aimor_handler();
            }break;
            
            default:
            {
            }break;
          }
        }
        else
        {
          memset(glb_cur.gimbal_cur,0,sizeof(glb_cur.gimbal_cur));
          gimbal.state = GIMBAL_INIT_NEVER;
        }
        
        pid_calc(&pid_yaw, gimbal.pid.yaw_angle_fdb, gimbal.pid.yaw_angle_ref);
        pid_calc(&pid_pit, gimbal.pid.pit_angle_fdb, gimbal.pid.pit_angle_ref);
        
        gimbal.pid.yaw_spd_ref = pid_yaw.out;
        gimbal.pid.pit_spd_ref = pid_pit.out;
        
        gimbal.pid.yaw_spd_fdb = gimbal.sensor.yaw_palstance;
        gimbal.pid.pit_spd_fdb = gimbal.sensor.pit_palstance;
        
        fuzzy_pid_calc(&pid_yaw_spd, gimbal.pid.yaw_spd_fdb, gimbal.pid.yaw_spd_ref);
        fuzzy_pid_calc(&pid_pit_spd, gimbal.pid.pit_spd_fdb, gimbal.pid.pit_spd_ref);
        /*自瞄 加前馈*/
        pit_ctrl_ffc = getFeedforwardControl(&p_ffc, gimbal.pid.pit_angle_ref) + pid_pit_spd.out;	
        yaw_ctrl_ffc = getFeedforwardControl(&y_ffc, gimbal.pid.yaw_angle_ref) + pid_yaw_spd.out;
        
        if (gimbal_is_controllable())
        {
          if(gimbal_mode == GIMBAL_TRACK_ARMOR)
          {
            glb_cur.gimbal_cur[0] = yaw_dir * yaw_ctrl_ffc;
            glb_cur.gimbal_cur[1] = pit_dir * pit_ctrl_ffc;
            glb_cur.gimbal_cur[2] = pid_trigger_spd.out;
          }
          else
          {
            glb_cur.gimbal_cur[0] = yaw_dir * pid_yaw_spd.out;
            glb_cur.gimbal_cur[1] = pit_dir * pid_pit_spd.out;
            glb_cur.gimbal_cur[2] = pid_trigger_spd.out;
          }
        }
        else
        {
          memset(glb_cur.gimbal_cur, 0, sizeof(glb_cur.gimbal_cur));
          gimbal_mode = GIMBAL_RELEASE;
          pid_trigger.iout = 0;
        }
      
        xTaskGenericNotify( (TaskHandle_t) can_msg_send_Task_Handle, 
                            (uint32_t) GIMBAL_MOTOR_MSG_SIGNAL, 
                            (eNotifyAction) eSetBits, 
                            (uint32_t *)NULL );
      }
    }
    
    gimbal_stack_surplus = uxTaskGetStackHighWaterMark(NULL);
  }
}


void gimbal_param_init(void)
{
  memset(&gimbal, 0, sizeof(gimbal_t));
  
  gimbal.state = GIMBAL_INIT_NEVER;
  
  ramp_init(&pit_ramp, 1000);
  ramp_init(&yaw_ramp, 1000);

	/* pit 轴电机的PID参数 */
	PID_Struct_Init(&pid_pit, pit_pid[0], pit_pid[1], pit_pid[2], 3000, 500, INIT);
  PID_Struct_Init(&pid_pit_spd, pit_pid[3], pit_pid[4], pit_pid[5], 6000, 4000, INIT);

  /* yaw 轴电机的PID参数 */
  PID_Struct_Init(&pid_yaw, yaw_pid[0], yaw_pid[1], yaw_pid[2], 5000, 500, INIT); 
  PID_Struct_Init(&pid_yaw_spd, yaw_pid[3] , yaw_pid[4], yaw_pid[5], 30000, 15000, INIT);  
  
  /* 拨盘 电机的PID参数 */
  PID_Struct_Init(&pid_trigger, trig_pid[0], trig_pid[1], trig_pid[2], 8000, 0, INIT);
  PID_Struct_Init(&pid_trigger_spd, trig_pid[3], trig_pid[4], trig_pid[5],8000, 3000,INIT);  
	
  /* ffc 初始化*/
	initFeedforwardParam(&p_ffc, 200, 10);
	initFeedforwardParam(&y_ffc, 200, 50);
  
}

static void init_mode_handler(void)
{
  /* PIT轴归中 */
  gimbal.pid.pit_angle_fdb = gimbal.sensor.pit_relative_angle;
  gimbal.pid.pit_angle_ref = gimbal.sensor.pit_relative_angle * (1 - ramp_calc(&pit_ramp));
  /* 保持YAW轴不动 */
  gimbal.pid.yaw_angle_fdb = gimbal.sensor.yaw_relative_angle;
  gimbal.pid.yaw_angle_ref = gimbal.sensor.yaw_relative_angle;
	
  if(gimbal.pid.pit_angle_fdb >= -3.0f && gimbal.pid.pit_angle_fdb <= 3.0f)
  {
    /*  偏航角轴归中*/
    gimbal.pid.yaw_angle_ref = gimbal.sensor.yaw_relative_angle * ( 1 - ramp_calc(&yaw_ramp));
    
    if (gimbal.pid.yaw_angle_fdb >= -3.0f && gimbal.pid.yaw_angle_fdb <= 3.0f)
    {
      gimbal.state = GIMBAL_INIT_DONE;
      
      gimbal.yaw_offset_angle = gimbal.sensor.yaw_gyro_angle;
      gimbal.pid.pit_angle_ref = 0;
//      gimbal.pid.yaw_angle_ref = 0;
      gimbal.pid.yaw_angle_ref = gimbal.sensor.yaw_relative_angle;
    }
  }
}

/*判断yaw轴是否有输入*/
static gimbal_state_t remote_is_action(void)
{
  if ((abs(rc.ch3) >= 10) || (abs(rc.mouse.x) >= 1))
  {
    return IS_ACTION;
  }
  else
  {
      return NO_ACTION;
  }
}

uint8_t input_flag;
uint32_t no_action_time;
uint32_t debug_time = 500;
static void nomarl_handler(void)
{ 
//  gimbal.state = remote_is_action(); //判断yaw轴是否有输入
//  if(gimbal.last_state == IS_ACTION && gimbal.state == NO_ACTION)
//  {
//    no_action_time = HAL_GetTick();
//  }
//  else if((gimbal.last_state == NO_ACTION) 
//        &&(gimbal.state == NO_ACTION)
//        &&(HAL_GetTick() - no_action_time > debug_time))
//  {
//    gimbal.yaw_offset_angle = gimbal.sensor.yaw_gyro_angle; //记录yaw轴陀螺仪位置
//    input_flag = 0;
//    gimbal.pid.yaw_angle_fdb = gimbal.sensor.yaw_relative_angle;
//    gimbal.pid.yaw_angle_ref = 0;
//  }
//  else
//  {
//    input_flag = 1;
//    gimbal.pid.yaw_angle_fdb = gimbal.sensor.yaw_gyro_angle - gimbal.yaw_offset_angle;
//    gimbal.pid.yaw_angle_ref += rm.yaw_v * GIMBAL_RC_MOVE_RATIO_YAW
//                              + km.yaw_v * GIMBAL_PC_MOVE_RATIO_YAW;
//  }
  
  if(last_gimbal_mode != GIMBAL_NORMAL_MODE)
    gimbal.pid.yaw_angle_ref = gimbal.sensor.yaw_relative_angle;
  
  gimbal.state = remote_is_action(); //判断yaw轴是否有输入
  if(gimbal.state == NO_ACTION)
  {
    gimbal.pid.yaw_angle_ref = gimbal.sensor.yaw_relative_angle;
    input_flag = 1;
  }
  else
    input_flag = 0;
  
  gimbal.pid.yaw_angle_fdb = gimbal.sensor.yaw_relative_angle;
  gimbal.pid.yaw_angle_ref += rm.yaw_v * GIMBAL_RC_MOVE_RATIO_YAW
                           + km.yaw_v * GIMBAL_PC_MOVE_RATIO_YAW;
  VAL_LIMIT(gimbal.pid.yaw_angle_ref, YAW_ANGLE_MIN, YAW_ANGLE_MAX);
  /*pitch轴*/
  gimbal.pid.pit_angle_fdb = gimbal.sensor.pit_relative_angle;
  gimbal.pid.pit_angle_ref += rm.pit_v * GIMBAL_RC_MOVE_RATIO_PIT
                         + km.pit_v * GIMBAL_PC_MOVE_RATIO_PIT;
  /* 软件限制pitch轴角度 */
  VAL_LIMIT(gimbal.pid.pit_angle_ref, PIT_ANGLE_MIN, PIT_ANGLE_MAX);

  
  gimbal.last_state = remote_is_action();//获取上次输入的状态
}

static void separate_handler(void)
{
  gimbal.state = remote_is_action(); //判断yaw轴是否有输入
  if(gimbal.state == NO_ACTION)
  {
    gimbal.yaw_offset_angle = gimbal.sensor.yaw_gyro_angle; //刷新陀螺仪yaw轴的零轴位置,抑制温漂
    gimbal.pid.yaw_angle_ref = 0; //yaw给定为0
  }
  
  gimbal.pid.pit_angle_fdb = gimbal.sensor.pit_relative_angle;
  gimbal.pid.yaw_angle_fdb = gimbal.sensor.yaw_gyro_angle - gimbal.yaw_offset_angle;
  
  gimbal.pid.yaw_angle_ref += rm.yaw_v * GIMBAL_RC_MOVE_RATIO_YAW
                         + km.yaw_v * GIMBAL_PC_MOVE_RATIO_YAW;
  
  /* 软件限制pitch轴角度 */
  if ((gimbal.sensor.pit_relative_angle >= PIT_ANGLE_MIN - 3) && \
      (gimbal.sensor.pit_relative_angle <= PIT_ANGLE_MAX + 3))
  {
		gimbal.pid.pit_angle_ref += rm.pit_v * GIMBAL_RC_MOVE_RATIO_PIT
													 + km.pit_v * GIMBAL_PC_MOVE_RATIO_PIT;
		
    VAL_LIMIT(gimbal.pid.pit_angle_ref, PIT_ANGLE_MIN, PIT_ANGLE_MAX);
  }
}

static void dodge_handler(void)
{ 
  if(last_gimbal_mode != GIMBAL_DODGE_MODE) //从跟随模式切换到小陀螺时，刷新零轴
  {
    gimbal.yaw_offset_angle = gimbal.sensor.yaw_gyro_angle;
    gimbal.pid.yaw_angle_ref = gimbal.yaw_offset_angle;
  }
  
  gimbal.state = remote_is_action(); //判断yaw轴是否有输入
  if(gimbal.last_state == NO_ACTION && gimbal.state == NO_ACTION)
  {
    gimbal.pid.yaw_angle_fdb = gimbal.sensor.yaw_gyro_angle;
    gimbal.pid.yaw_angle_ref = gimbal.yaw_offset_angle;
  }
  else if(gimbal.last_state == IS_ACTION && gimbal.state == NO_ACTION)
  {
    gimbal.yaw_offset_angle = gimbal.sensor.yaw_gyro_angle;
  }
  else
  {
    gimbal.pid.yaw_angle_fdb = gimbal.sensor.yaw_gyro_angle;
    gimbal.pid.yaw_angle_ref += rm.yaw_v * GIMBAL_RC_MOVE_RATIO_YAW
                              + km.yaw_v * GIMBAL_PC_MOVE_RATIO_YAW;
  }
  
  /*pitch轴*/
  gimbal.pid.pit_angle_fdb = gimbal.sensor.pit_relative_angle;
  gimbal.pid.pit_angle_ref += rm.pit_v * GIMBAL_RC_MOVE_RATIO_PIT
                         + km.pit_v * GIMBAL_PC_MOVE_RATIO_PIT;
  /* 软件限制pitch轴角度 */
  VAL_LIMIT(gimbal.pid.pit_angle_ref, PIT_ANGLE_MIN, PIT_ANGLE_MAX);
  
  gimbal.last_state = remote_is_action();//获取上次输入的状态
}  

static void track_aimor_handler(void)
{
	/* 记录丢失角度 */
  float lost_pit;
	float lost_yaw;
	
  float yaw_ctrl;
  float pit_ctrl;
	uint8_t  last_vision_status;
	
	gimbal.yaw_offset_angle = gimbal.sensor.yaw_gyro_angle;//备份陀螺仪数据，以免退出自瞄时冲突
  gimbal.pid.pit_angle_fdb = gimbal.sensor.pit_relative_angle;
  gimbal.pid.yaw_angle_fdb = gimbal.sensor.yaw_gyro_angle;//yaw轴用陀螺仪,以免跟小陀螺冲突
			
	if (pc_recv_mesg.gimbal_control_data.visual_valid == 1)
	{
		last_vision_status = 1;
    
		yaw_ctrl = gimbal.sensor.yaw_gyro_angle + pc_recv_mesg.gimbal_control_data.yaw_ref;
 	  pit_ctrl = gimbal.sensor.pit_relative_angle + pc_recv_mesg.gimbal_control_data.pit_ref;
	}	
	
	/*视觉无效处理*/
  else if (pc_recv_mesg.gimbal_control_data.visual_valid == 0)		
	{
    if (last_vision_status == 1)
    {			
			lost_pit = gimbal.sensor.pit_relative_angle;//丢失目标后云台不动
			lost_yaw = gimbal.sensor.yaw_relative_angle;			
			
			last_vision_status = 2;
    }		
			pit_ctrl = lost_pit;
		  yaw_ctrl = lost_yaw;
	}
	
	gimbal.pid.yaw_angle_ref = yaw_ctrl;
	gimbal.pid.pit_angle_ref = pit_ctrl;
	
  /*给定角度限制*/		
	if (chassis_mode != CHASSIS_DODGE_MODE)
	{
  	VAL_LIMIT(gimbal.pid.yaw_angle_ref, -45, 45);
	  VAL_LIMIT(gimbal.pid.pit_angle_ref, -25, 25);
	}
}

static void shoot_buff_ctrl_handler(void)
{
	/*记录丢失角度*/
	static float lost_pit;
	static float lost_yaw;
	
	/*控制角度*/
  static float yaw_ctrl;
  static float pit_ctrl;
	
	static uint8_t  last_vision_status;
	
	/*获取反馈值*/
	gimbal.yaw_offset_angle  = gimbal.sensor.yaw_gyro_angle; //备份陀螺仪数据，以免退出自瞄时冲突
  gimbal.pid.pit_angle_fdb = gimbal.sensor.pit_relative_angle;
  gimbal.pid.yaw_angle_fdb = gimbal.sensor.yaw_relative_angle;
  
	if(pc_recv_mesg.gimbal_control_data.visual_valid == 1)
	{
		last_vision_status = 1;
		
		yaw_ctrl = pc_recv_mesg.gimbal_control_data.yaw_ref;
 	  pit_ctrl = pc_recv_mesg.gimbal_control_data.pit_ref;
	}	
  
	/*视觉无效处理*/
  else if(pc_recv_mesg.gimbal_control_data.visual_valid == 0)		
	{
    if (last_vision_status == 1)
    {			
			lost_pit = gimbal.sensor.pit_relative_angle;//丢失目标后云台不动
			lost_yaw = gimbal.sensor.yaw_relative_angle;			
			
			last_vision_status = 2;
    }		
			pit_ctrl = lost_pit;
		  yaw_ctrl = lost_yaw;
	}
	
	gimbal.pid.yaw_angle_ref = yaw_ctrl;
	gimbal.pid.pit_angle_ref = pit_ctrl;
	
  /*给定角度限制*/	
 	VAL_LIMIT(gimbal.pid.yaw_angle_ref, -45, 45);
	VAL_LIMIT(gimbal.pid.pit_angle_ref, -25, 25);
}

float speed_threshold = 10.0f;
//time_raw,pitch_angel_raw等都来自pc
//此函数可算出速度，调用时position实参为pitch_angel_raw
//函数实为给结构体speed_calc_data_t赋值，初始化参数
float target_speed_calc(speed_calc_data_t *S, uint32_t time, float position)
{
  S->delay_cnt++;

  if (time != S->last_time)
  {
    S->speed = (position - S->last_position) / (time - S->last_time) * 1000;		//计算速度（当前位置-上次位置）/
#if 1
    if ((S->speed - S->processed_speed) < -speed_threshold)		//speed_threshold=10.0f,S->processed_speed未赋过值
    {
        S->processed_speed = S->processed_speed - speed_threshold;
    }
    else if ((S->speed - S->processed_speed) > speed_threshold)
    {
        S->processed_speed = S->processed_speed + speed_threshold;
    }
    else 
#endif
      S->processed_speed = S->speed;													//将前面计算出的速度赋予S->processed_speed
    
    S->last_time = time;
    S->last_position = position;
    S->last_speed = S->speed;
    S->delay_cnt = 0;
  }
  
  if(S->delay_cnt > 200) // delay 200ms speed = 0
  {
    S->processed_speed = 0;
  }

  return S->processed_speed;
}

void initFeedforwardParam(FFC *vFFC,float a,float b)
{
	//初始化前馈补偿
	vFFC->a = a;
	vFFC->b = b;
	vFFC->lastRin = 0;
	vFFC->perrRin = 0;
	vFFC->rin = 0;
}

/*实现前馈控制器*/
float getFeedforwardControl(FFC* vFFC,float v)//yaw轴
{
	vFFC->rin = v;
	float result = vFFC->a * (vFFC->rin - vFFC->lastRin) + vFFC->b * (vFFC->rin - 2 * vFFC->lastRin + vFFC->perrRin);
	vFFC->perrRin = vFFC->lastRin;
	vFFC->lastRin = vFFC->rin;
	return result;
}
