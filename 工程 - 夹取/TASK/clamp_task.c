#include "clamp_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modeswitch_task.h"
#include "comm_task.h"
#include "detect_task.h"
#include "upraise_task.h"
#include "pid.h"
#include "bsp_can.h"
#include "math.h"

UBaseType_t clamp_stack_surplus;
extern TaskHandle_t can_msg_send_Task_Handle;

clamp_t clamp;
pick_box_t pick_box;
island_offset offset_angle;

float clamp_pid[6] = {15, 15, 0, 5, 15, 0};

void clamp_task(void *parm)
{
	uint32_t Signal;
	BaseType_t STAUS;
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
										        (uint32_t) INFO_GET_CLAMP_SIGNAL, 
									        	(uint32_t *)&Signal, 
									        	(TickType_t) portMAX_DELAY );
		if(STAUS == pdTRUE)
		{
			if(Signal & INFO_GET_CLAMP_SIGNAL)
			{
        for(int i = 0; i < 3; i++)
        {
          PID_Struct_Init(&pid_clamp[i],clamp_pid[0],clamp_pid[1],clamp_pid[2],5000, 500, INIT);
          PID_Struct_Init(&pid_clamp_spd[i],clamp_pid[3],clamp_pid[4],clamp_pid[5],10000, 500, INIT);
        }
        
        if(global_mode == GLOBAL_CLAMP_MODE && upraise.updown_flag == UP)
        {
          if(clamp.state == INIT_NEVER)
          {
             clamp_mode = CLAMP_INIT;
          }
          
          switch(clamp_mode)
          {
            case CLAMP_INIT:
            {
              clamp_init_handler();
            }break;
            
            case SMALL_ISLAND:
            {
              small_island_handler();
            }break;
            
            case BIG_ISLAND:
            {
              big_island_handler();
            }break;
            
            case AUTOMATIC_SMALL_ISLAND:
            {
              auto_small_island_handler();
            }break;
            
            case AUTOMATIC_BIG_ISLAND:
            {
              auto_big_island_handler();
            }break;
            
            default:
            {
              
            }break;
            
          }
          
        }
        else
        {
          clamp.clamp_cmd = 0;
          clamp.c_clamp_cmd = 0;
        }
        
        clamp.last_state = clamp.state; //获取上一次状态
        
        if(!clamp_is_controllable())
        {
          if(clamp.state == INIT_DONE)
          {
            clamp.angle_ref[0] = clamp.init_angle[0];
            clamp.angle_ref[1] = clamp.init_angle[1];
            clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.small_island.offset_angle_mid; //回到中间
          }
          else
          {
            for(int i=0;i<3;i++)
            {
              clamp.spd_ref[i] = 0;
            }
          }
        }
        
        for(int i = 0;i < 3;i++)
        {
          pid_calc(&pid_clamp[i], moto_clamp[i].total_angle, clamp.angle_ref[i]);
          if(clamp.state == INIT_NEVER)
            pid_calc(&pid_clamp_spd[i], clamp.spd_fdb[i], clamp.spd_ref[i]);
          else
            pid_calc(&pid_clamp_spd[i], clamp.spd_fdb[i], pid_clamp[i].out);
        }
        
        xTaskGenericNotify( (TaskHandle_t) can_msg_send_Task_Handle, 
                          (uint32_t) CLAMP_MOTOR_MSG_SIGNAL, 
                          (eNotifyAction) eSetBits, 
                          (uint32_t *)NULL );
        
      }
    }
    
    clamp_stack_surplus = uxTaskGetStackHighWaterMark(NULL);       
  }
}


void clamp_param_init(void)
{
  memset(&clamp, 0, sizeof(clamp_t));
  
  clamp.init_flag = 0;
  clamp.state = INIT_NEVER;
  clamp.last_state = INIT_DONE;
  clamp.clamp_flag = CLAMPED;

  pick_box = FRIST_BOX; //第一个位置状态
  
	for(int i = 0; i < 3; i++)
  {
    PID_Struct_Init(&pid_clamp[i],clamp_pid[0],clamp_pid[1],clamp_pid[2],5000, 500, INIT);
    PID_Struct_Init(&pid_clamp_spd[i],clamp_pid[3],clamp_pid[4],clamp_pid[5],10000, 500, INIT);
//    clamp.init_angle[i] = moto_clamp[i].total_angle;
  }
  
  offset_angle.small_island.offset_angle_left   = 0;    //左
  offset_angle.small_island.offset_angle_mid    = 1000; //中间
  offset_angle.small_island.offset_angle_right  = 2000; //右
  
  offset_angle.big_island.offset_angle_left     = 0;    //左
  offset_angle.big_island.offset_angle_mid      = 1000; //中间
  offset_angle.big_island.offset_angle_right    = 2000; //右
  
  GPIO_ResetBits(GPIOB,GPIO_Pin_0); //气缸缩回来
  GPIO_SetBits(GPIOE,GPIO_Pin_12);
  
  GPIO_ResetBits(GPIOE,GPIO_Pin_4); //夹子松开
  GPIO_SetBits(GPIOF,GPIO_Pin_0);
  
}

/*
*  error_angle[3]      : 记录每个电机误差的次数，用来判断电机是否堵转
*  error_state[3]      : 记录每个电机的状态，是否已经堵转
*  clamp.init_flag     ：为了某电机出现堵转后就停下来，且不影响其他电机
*
*/
void clamp_init_handler(void)
{
  static uint32_t error_angle[3] = {0};
  static uint8_t error_state[3] = {0};
  if((clamp.init_flag == 0) || (clamp.state != clamp.last_state))
  {
    clamp.spd_ref[0] = 500;//左边
    clamp.spd_ref[1] = -500;//右边
    clamp.spd_ref[2] = 500;//左平移
  }
  for(int i=0;i<3;i++)
  {
    if((fabs(pid_clamp_spd[i].set) - fabs(pid_clamp_spd[i].get)) > 0.7f*fabs(pid_clamp_spd[i].set))
    {
      error_angle[i]++;
      if(error_angle[i] > 250)
      {
        error_state[i] = 1;
        clamp.spd_ref[i] = 0;
        clamp.init_flag = 1;
      }
    }
  }
  if(error_state[0] == 1 && error_state[1] == 1 && error_state[2] == 1)
  {
    for(int i=0;i<3;i++)
    {
      clamp.init_angle[i] = moto_clamp[i].total_angle;
      error_angle[i] = 0;
      error_state[i] = 0;
      clamp.state = INIT_DONE;
      clamp.init_flag = 0;
    }
  }
}


uint32_t action_times;
void small_island_handler(void)
{
  if(clamp.clamp_cmd)
  {
    clamp.clamp_flag = CLAMPING;
    if(pick_box == FRIST_BOX)
    {
      if(HAL_GetTick() - action_times > 10)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
      }
      if(HAL_GetTick() - action_times > 200)
      {
        GPIO_ResetBits(GPIOF,GPIO_Pin_0); //夹住
        GPIO_SetBits(GPIOE,GPIO_Pin_4);
      }
      if(HAL_GetTick() - action_times > 400)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 45;  //向上翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 45;
      }
      if(HAL_GetTick() - action_times > 900)
      {
        clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.small_island.offset_angle_left; //往左移
      }
      if(HAL_GetTick() - action_times > 1100)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
      }
      if(HAL_GetTick() - action_times > 1250)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_4); //松开
        GPIO_SetBits(GPIOF,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 1300)
      {
        action_times = HAL_GetTick();
        pick_box = SECOND_BOX;
        clamp.clamp_cmd = 0;
      }
    }
  }
  if(clamp.c_clamp_cmd && pick_box != FRIST_BOX)
  {
    /*第2箱*/
    if(pick_box == SECOND_BOX)
    {
      if(HAL_GetTick() - action_times > 100)
      {
        GPIO_ResetBits(GPIOF,GPIO_Pin_0); //夹住
        GPIO_SetBits(GPIOE,GPIO_Pin_4);
      }
      if(HAL_GetTick() - action_times > 150)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 45;  //向上翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 45;
      }
      if(HAL_GetTick() - action_times > 300)
      {
        clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.small_island.offset_angle_mid; //回中
      }
       if(HAL_GetTick() - action_times > 650)
      {
        clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.small_island.offset_angle_right; //往右移
      }
      if(HAL_GetTick() - action_times > 850)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
      }
      if(HAL_GetTick() - action_times > 1000)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_4); //松开
        GPIO_SetBits(GPIOF,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 1050)
      {
        pick_box = THRID_BOX;
        action_times = HAL_GetTick();
      }

    }
    /*第3箱*/
    if(pick_box == THRID_BOX)
    {
      if(HAL_GetTick() - action_times > 50)
      {
        GPIO_ResetBits(GPIOF,GPIO_Pin_0); //夹住
        GPIO_SetBits(GPIOE,GPIO_Pin_4);
      }
      if(HAL_GetTick() - action_times > 150)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 45;  //向上翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 45;
      }
      if(HAL_GetTick() - action_times > 350)
      {
        clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.small_island.offset_angle_mid; //回中
      }
      if(HAL_GetTick() - action_times > 600)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
      }
      if(HAL_GetTick() - action_times > 750)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_4); //松开
        GPIO_SetBits(GPIOF,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 800)
      {
        pick_box = FRIST_BOX;
        action_times = HAL_GetTick();
      }
    }
  }
  if(clamp.clamp_cmd == 0 && clamp.c_clamp_cmd == 0)
  {
    action_times = HAL_GetTick();
    pick_box = FRIST_BOX;
    clamp.clamp_flag = CLAMPED;
    clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.big_island.offset_angle_mid; //回中
    clamp.angle_ref[0] = clamp.init_angle[0] + 100;  //向下翻转
    clamp.angle_ref[1] = clamp.init_angle[1] - 100;
  }
}

void big_island_handler(void)
{
  if(clamp.clamp_cmd)
  {
    clamp.clamp_flag = CLAMPING;
    if(pick_box == FRIST_BOX)
    {
      if(HAL_GetTick() - action_times > 10)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
      }
      if(HAL_GetTick() - action_times > 200)
      {
        GPIO_ResetBits(GPIOF,GPIO_Pin_0); //夹住
        GPIO_SetBits(GPIOE,GPIO_Pin_4);
      }
      if(HAL_GetTick() - action_times > 300)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 45;  //向上翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 45;
      }
      if(HAL_GetTick() - action_times > 600)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_12); //气缸伸出去
        GPIO_SetBits(GPIOB,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 750)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
      }
      if(HAL_GetTick() - action_times > 900)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_4); //松开
        GPIO_SetBits(GPIOF,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 950)
      {
        action_times = HAL_GetTick();
        pick_box = SECOND_BOX;
        clamp.clamp_cmd = 0;
      }
    }
  }
  if(clamp.c_clamp_cmd && pick_box != FRIST_BOX)
  {
    /*第2箱*/
    if(pick_box == SECOND_BOX)
    {
//      if(HAL_GetTick() - action_times > 10)
//      {
//        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
//        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
//      }
      if(HAL_GetTick() - action_times > 50)
      {
        GPIO_ResetBits(GPIOF,GPIO_Pin_0); //夹住
        GPIO_SetBits(GPIOE,GPIO_Pin_4);
      }
      if(HAL_GetTick() - action_times > 150)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 45;  //向上翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 45;
      }
      if(HAL_GetTick() - action_times > 250)
      {
        GPIO_ResetBits(GPIOB,GPIO_Pin_0); //气缸缩回来
        GPIO_SetBits(GPIOE,GPIO_Pin_12);
      }
      if(HAL_GetTick() - action_times > 450)
      {
        clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.big_island.offset_angle_left; //左移
      }
      if(HAL_GetTick() - action_times > 700)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_12); //气缸伸出去
        GPIO_SetBits(GPIOB,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 800)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
      }
      if(HAL_GetTick() - action_times > 950)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_4); //松开
        GPIO_SetBits(GPIOF,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 1000)
      {
        pick_box = THRID_BOX;
        action_times = HAL_GetTick();
      }

    }
    /*第3箱*/
    if(pick_box == THRID_BOX)
    {
      if(HAL_GetTick() - action_times > 50)
      {
        GPIO_ResetBits(GPIOF,GPIO_Pin_0); //夹住
        GPIO_SetBits(GPIOE,GPIO_Pin_4);
      }
      if(HAL_GetTick() - action_times > 150)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 45;  //向上翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 45;
      }
      if(HAL_GetTick() - action_times > 350)
      {
        GPIO_ResetBits(GPIOB,GPIO_Pin_0); //气缸缩回来
        GPIO_SetBits(GPIOE,GPIO_Pin_12);
      }
      if(HAL_GetTick() - action_times > 450)
      {
        clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.big_island.offset_angle_mid; //回中
      }
      if(HAL_GetTick() - action_times > 650)
      {
        clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.big_island.offset_angle_right; //右移
      }
      if(HAL_GetTick() - action_times > 950)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_12); //气缸伸出去
        GPIO_SetBits(GPIOB,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 1050)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
      }
      if(HAL_GetTick() - action_times > 1200)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_4); //松开
        GPIO_SetBits(GPIOF,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 1250)
      {
        pick_box = FOURTH_BOX;
        action_times = HAL_GetTick();
      }
    }
    /*第4箱*/
    if(pick_box == FOURTH_BOX)
    {
      if(HAL_GetTick() - action_times > 50)
      {
        GPIO_ResetBits(GPIOF,GPIO_Pin_0); //夹住
        GPIO_SetBits(GPIOE,GPIO_Pin_4);
      }
      if(HAL_GetTick() - action_times > 150)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 45;  //向上翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 45;
      }
      if(HAL_GetTick() - action_times > 350)
      {
        GPIO_ResetBits(GPIOB,GPIO_Pin_0); //气缸缩回来
        GPIO_SetBits(GPIOE,GPIO_Pin_12);
      }
      if(HAL_GetTick() - action_times > 550)
      {
        clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.big_island.offset_angle_mid; //回中
      }
      if(HAL_GetTick() - action_times > 850)
      {
        clamp.angle_ref[0] = clamp.init_angle[0] + 180;  //向下翻转
        clamp.angle_ref[1] = clamp.init_angle[1] - 180;
      }
      if(HAL_GetTick() - action_times > 1000)
      {
        GPIO_ResetBits(GPIOE,GPIO_Pin_4); //松开
        GPIO_SetBits(GPIOF,GPIO_Pin_0);
      }
      if(HAL_GetTick() - action_times > 1050)
      {
        pick_box = FRIST_BOX;
        action_times = HAL_GetTick();
      }
    }
  }
  if(clamp.clamp_cmd == 0 && clamp.c_clamp_cmd == 0)
  {
    action_times = HAL_GetTick();
    pick_box = FRIST_BOX;
    clamp.clamp_flag = CLAMPED;
    clamp.angle_ref[2] = clamp.init_angle[2] + offset_angle.big_island.offset_angle_mid; //回中
    clamp.angle_ref[0] = clamp.init_angle[0] + 100;  //向下翻转
    clamp.angle_ref[1] = clamp.init_angle[1] - 100;
  }
}

void auto_small_island_handler(void)
{
  
}

void auto_big_island_handler(void)
{
  
}
