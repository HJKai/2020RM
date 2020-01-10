#include "shoot_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "comm_task.h"
#include "modeswitch_task.h"
#include "detect_task.h"
#include "string.h"
#include "sys_config.h"
#include "math.h"
#include "pid.h"
#include "bsp_can.h"
#include "judge_rx_data.h"

UBaseType_t shoot_stack_surplus;
extern TaskHandle_t can_msg_send_Task_Handle;

shoot_t   shoot;
trigger_t trig;

#if (INFANTRY_NUM == INFANTRY_1)

  /*弹仓盖开关*/
  float ccr_open = 500;
  float ccr_close = 1300;
	/* 摩擦轮转速 */
	uint16_t mad_speed	 		= 1190;		
	uint16_t normal_speed		= 1150;   
	uint16_t throw_speed	 	= 1325;		
	uint16_t buff_speed			= 1350;		
	
	/* 拨盘转速 */
	int normal_cshoot				= 1000;		
	
	/*顺序：普通-基地-吊射-buff*/
	/*热量限制拨盘转速*/
  float heat_limit_pid[3] = {30, 0, 10};
  /*摩擦轮pid*/
  float fric_pid[3] = {10, 0, 0};
	
#else
		#error "INFANTRY_NUM define error!"
#endif
	

void shoot_task(void *parm)
{
  uint32_t Signal;
	BaseType_t STAUS;
  
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
										        (uint32_t) INFO_GET_SHOOT_SIGNAL, 
									        	(uint32_t *)&Signal, 
									        	(TickType_t) portMAX_DELAY );
    if(STAUS == pdTRUE)
		{
			if(Signal & INFO_GET_SHOOT_SIGNAL)
			{
        if(shoot_mode != SHOOT_DISABLE)
        {
          /*热量控制*/
          PID_Struct_Init(&pid_heat_limit, heat_limit_pid[0], heat_limit_pid[1], heat_limit_pid[2], 7000, 0, DONE);
          
          /*摩擦轮*/
          for(int i=0;i<2;i++)
          {
            PID_Struct_Init(&pid_fric[i], fric_pid[0], fric_pid[1], fric_pid[2], 8000, 500, DONE); 
          }
          
//          if(last_shoot_mode == SHOOT_DISABLE)
//          {
//            trig.angle_ref = moto_trigger.total_ecd; //记录当前拨盘电机编码位
//          }
          
          shoot_para_ctrl();						// 射击模式切换
          ball_storage_ctrl();					// 舵机控制弹仓盖
          fric_wheel_ctrl();						// 启动摩擦轮
          
          if (shoot.fric_wheel_run)
          {
            shoot_bullet_handler();           
          }
          else
          {
            shoot.shoot_cmd   = 0;
            shoot.c_shoot_cmd = 0;
            shoot.fric_wheel_spd = 0;
            pid_trigger_spd.out = 0;
            trig.angle_ref = moto_trigger.total_angle; //记录当前拨盘电机编码位
          }
          
          get_last_shoot_mode();
        }
        else
        {
          pid_trigger_spd.out = 0;
          shoot.fric_wheel_spd = 0;
        }
        xTaskGenericNotify( (TaskHandle_t) can_msg_send_Task_Handle, 
                          (uint32_t) SHOT_MOTOR_MSG_SIGNAL, 
                          (eNotifyAction) eSetBits, 
                          (uint32_t *)NULL );
      }
    }
			
    shoot_stack_surplus = uxTaskGetStackHighWaterMark(NULL);
	}
		
} 



void get_last_shoot_mode(void)
{
	shoot.last_para_mode = shoot.para_mode;
}


/*防卡弹*/
int32_t debug_error = 500/*检测卡弹的灵敏度*/,debug_ref = -5500/*卡弹回拨参数*/;
uint32_t trig_error;
void block_bullet_handler(void)
{
	static uint32_t stall_mark = 0;
	static uint32_t stall_step = 0;
	
  if(((fabs(pid_trigger_spd.set) - fabs(pid_trigger_spd.get)) > 0.8f*fabs(pid_trigger_spd.set)) && stall_step == 0)
    trig_error++;
  else
    trig_error = 0;

	if((stall_step == 0) && (trig_error > debug_error))	//判断是否卡弹
	{	
		stall_step = 1;
		stall_mark = HAL_GetTick();
	}
	if(stall_step == 1)
	{
		trig.spd_ref = debug_ref;
		if(HAL_GetTick() - stall_mark >= 150)//150ms
		{
			stall_step = 0;
			stall_mark = 0;
			trig_error = 0;
		}
	}
}

/*射击模式选择
* @ SHOOTBUFF_MODE 神符模式
*	@ SHOOTTHROW_MODE 高射速吊射模式
*	@ SHOOTMAD_MODE 低射速近战模式
* @ SHOOTNOR_MODE 普通射速模式
**/
static void shoot_para_ctrl(void)
{
	switch(shoot.para_mode)
	{
		case SHOOTBUFF_MODE:
    {
				shoot.fric_wheel_spd = buff_speed;
		}break;
		
		case SHOOTTHROW_MODE:
    {
			 shoot.fric_wheel_spd = throw_speed;
		}break;
		
		case SHOOTMAD_MODE:
    {
			 shoot.fric_wheel_spd = mad_speed;
		}break;
		
		default:
    {
			 shoot.fric_wheel_spd = normal_speed;	
		}break;
	}
	
	/* 拨盘转速 */
	trig.shoot_spd			 = 3000;
	trig.c_shoot_spd		 = normal_cshoot;
}

/*摩擦轮控制*/
static void fric_wheel_ctrl(void)
{
	if (shoot.fric_wheel_run)
	{
		turn_on_friction_wheel(shoot.fric_wheel_spd, shoot.fric_wheel_spd);
	}
	else
	{
		turn_off_friction_wheel();
	}
}

/*打开摩擦轮*/
static void turn_on_friction_wheel(int16_t lspd,int16_t rspd)
{
  pid_calc(&pid_fric[0], moto_fric[0].speed_rpm, lspd);
	pid_calc(&pid_fric[1], moto_fric[1].speed_rpm, rspd);
	glb_cur.fric_cur[0] = pid_fric[0].out;
	glb_cur.fric_cur[1] = pid_fric[1].out;
}

/*关闭摩擦轮*/
static void turn_off_friction_wheel(void)
{
	pid_calc(&pid_fric[0], moto_fric[0].speed_rpm, 0);
	pid_calc(&pid_fric[1], moto_fric[1].speed_rpm, 0);
	glb_cur.fric_cur[0] = pid_fric[0].out;
	glb_cur.fric_cur[1] = pid_fric[1].out;
}

/*弹仓盖控制*/
static void ball_storage_ctrl(void)
{
  if (shoot.ball_storage_open)
  {
    TIM5->CCR4 = ccr_open;
  }
  else
  {
    TIM5->CCR4 = ccr_close;
  }
}


static void shoot_bullet_handler(void)	//没裁判系统
{
  float suplus_heat;
  
//  trig.angle_ref = moto_trigger.total_ecd; //记录当前拨盘电机编码位
  
  if (shoot.shoot_cmd)//单发
  {
    trig.angle_ref = moto_trigger.total_angle + 45;//一发转45度
    shoot.shoot_cmd = 0;
  }
  
  pid_calc(&pid_trigger,moto_trigger.total_angle,trig.angle_ref);
                    
  if(!global_err.list[JUDGE_SYS_OFFLINE].err_exist && judge_recv_mesg.power_heat_data.shooter_heat0 
      > judge_recv_mesg.game_robot_state.shooter_heat0_cooling_limit - 100)
    trig.spd_ref = 0;
  else
    trig.spd_ref = pid_trigger.out;

  if (shoot.c_shoot_cmd)//连发处理
  {
    if ( !global_err.list[JUDGE_SYS_OFFLINE].err_exist )
		{//裁判系统在线模式
      suplus_heat = (judge_recv_mesg.game_robot_state.shooter_heat0_cooling_limit - 30) \
                     - judge_recv_mesg.power_heat_data.shooter_heat0;
      fuzzy_pid_calc(&pid_heat_limit,suplus_heat,0);
      trig.spd_ref = -pid_heat_limit.out;
	  }		
		else 
		{
			trig.spd_ref = trig.c_shoot_spd;
		}
    trig.angle_ref = moto_trigger.total_angle;
  }

	    /* 卡弹处理 */
  block_bullet_handler();
  
  pid_calc(&pid_trigger_spd, moto_trigger.speed_rpm, trig.spd_ref);
	
	if(trig.spd_ref == 0)
	{
		pid_trigger_spd.out = 0;
	}
}

void shoot_param_init(void)
{
  memset(&shoot, 0, sizeof(shoot_t));
  
  shoot.ctrl_mode      = SHOT_DISABLE;
	shoot.para_mode			 = SHOOTNOR_MODE;
  
  memset(&trig, 0, sizeof(trigger_t));
  
  trig.shoot_spd			 = 0;
  trig.c_shoot_spd     = 0;
  trig.one_sta         = TRIG_INIT;
  
  /*热量控制*/
  PID_Struct_Init(&pid_heat_limit, heat_limit_pid[0], heat_limit_pid[1], heat_limit_pid[2], 7000, 0, INIT);
  
  /*摩擦轮*/
  for(int i=0;i<2;i++)
  {
    PID_Struct_Init(&pid_fric[i], fric_pid[0], fric_pid[1], fric_pid[2], 8000, 500, INIT); 
  }
  
}


