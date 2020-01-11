#include "keyboard.h"
#include "remote_ctrl.h"
#include "STM32_TIM_BASE.h"
#include "sys_config.h"
#include "ramp.h"
#include "chassis_task.h"
#include "gimbal_task.h"
#include "shoot_task.h"


kb_ctrl_t km;
ramp_t key_fbramp;
ramp_t key_rlramp;

void get_mouse_status(MOUSE_STATUS *status,uint8_t mouse)
{
  switch(*status)
  {
    case MOUSE_RELEASE:
    {
      if(mouse)
        *status = MOUSE_PRESS;
      else
        *status = MOUSE_RELEASE;
    }break;
    
    case MOUSE_PRESS:
    {
      if(mouse)
        *status = MOUSE_DONE;
      else
        *status = MOUSE_RELEASE;
    }break;
    
    case MOUSE_DONE:
    {
      if(mouse)
      {
        *status = MOUSE_ONCE;
        if(status == &km.l_mouse_sta)
          km.l_cnt = HAL_GetTick();
        else
          km.r_cnt = HAL_GetTick();
      }
      else
        *status = MOUSE_RELEASE;
    }break;
    
    case MOUSE_ONCE:
    {      
      if(mouse)
      {
        if(status == &km.l_mouse_sta)
        {
          if(HAL_GetTick() - km.l_cnt > 100)
            *status = MOUSE_LONG;
        }
        else
        {
          if(HAL_GetTick() - km.r_cnt > 100)
            *status = MOUSE_LONG;
        }
      }
      else
        *status = MOUSE_RELEASE;
    }break;
    
    case MOUSE_LONG:
    {
      if(!mouse)
      {
        *status = MOUSE_RELEASE;
      }
    }break;
    
    default:
    {
    }break;
  }
}

void keyboard_global_hook(void)
{
  if (km.kb_enable)
  {
    get_mouse_status(&km.l_mouse_sta, rc.mouse.l);
    get_mouse_status(&km.r_mouse_sta, rc.mouse.r);
  }
}

/*控制速度*/
static void chassis_speed_ctrl(uint8_t fast,uint8_t slow)
{
  if(slow)//慢速
  {
    km.vx_limit_speed = 0.5f * CHASSIS_KB_MOVE_RATIO_X * CHASSIS_KB_MAX_SPEED_X;
    km.vy_limit_speed = 0.5f * CHASSIS_KB_MOVE_RATIO_Y * CHASSIS_KB_MAX_SPEED_Y;
  }
  else if(fast)//快速
  {
    km.vx_limit_speed = 1.5f * CHASSIS_KB_MOVE_RATIO_X * CHASSIS_KB_MAX_SPEED_X;
    km.vy_limit_speed = 1.5f * CHASSIS_KB_MOVE_RATIO_Y * CHASSIS_KB_MAX_SPEED_Y;
  }
  else  //正常速度
  {
    km.vx_limit_speed = CHASSIS_KB_MOVE_RATIO_X * CHASSIS_KB_MAX_SPEED_X;
    km.vy_limit_speed = CHASSIS_KB_MOVE_RATIO_Y * CHASSIS_KB_MAX_SPEED_Y;
  }
}

/*控制方向*/
static void chassis_direction_ctrl(uint8_t forward, uint8_t back,
                                uint8_t left,    uint8_t right)
{
  if(forward)
  {
    km.vx = CHASSIS_KB_MOVE_RATIO_X * km.vx_limit_speed * ramp_calc(&key_fbramp);
  }
  else if(back)
  {
    km.vx = CHASSIS_KB_MOVE_RATIO_X * (-km.vx_limit_speed) * ramp_calc(&key_fbramp);
  }
  else
  {
    km.vx = 0;
    ramp_init(&key_fbramp,300);
  }
  
  if(left)
  {
    km.vy = CHASSIS_KB_MOVE_RATIO_Y * km.vy_limit_speed * ramp_calc(&key_rlramp);
  }
  else if(right)
  {
    km.vy = CHASSIS_KB_MOVE_RATIO_Y * (-km.vy_limit_speed) * ramp_calc(&key_rlramp);
  }
  else
  {
    km.vy = 0;
    ramp_init(&key_rlramp,300);
  }
}


/*控制摩擦轮*/
static void firc_ctrl(uint8_t firc_open,uint8_t firc_close)
{
  if(firc_open)
  {
    shoot.fric_wheel_run = 1;
  }
  if(firc_close)
  {
    shoot.fric_wheel_run = 0;
  }
}

/*控制单发和连发命令*/
static void shoot_cmd_ctrl(uint8_t shoot_cmd,uint8_t c_shoot_cmd)
{
  if(shoot_cmd)
  {
    shoot.shoot_cmd = 1;
    shoot.c_shoot_cmd = 0;
  }
  if(c_shoot_cmd)
  {
    shoot.shoot_cmd = 0;
    shoot.c_shoot_cmd = 1;
  }
  else
  {
    shoot.c_shoot_cmd = 0;
  }
}

/*控制弹舱盖*/
static void shoot_storage_ctrl(uint8_t storage_open,uint8_t storage_close)
{
  if(storage_open)
  {
    shoot.ball_storage_open = 1;
  }
  if(storage_close)
  {
    shoot.ball_storage_open = 0;
  }
}


static void gimbal_speed_ctrl(int16_t pit_ref_spd, int16_t yaw_ref_spd)
{
  km.pit_v = pit_ref_spd * 0.01 * GIMBAL_PC_MOVE_RATIO_PIT;
  km.yaw_v = yaw_ref_spd * 0.01 * GIMBAL_PC_MOVE_RATIO_YAW;
}

void keyboard_chassis_hook(void)
{
  if(km.kb_enable)
  {
    chassis_speed_ctrl(FAST_SPD, SLOW_SPD);
    chassis_direction_ctrl(FORWARD, BACK, LEFT, RIGHT);
  }
  else
  {
    km.vx = 0;
    km.vy = 0;
  }
}

void keyboard_shoot_hook(void)
{
  if(km.kb_enable)
  {
    firc_ctrl(KB_OPEN_FRIC_WHEEL, KB_CLOSE_FIRC_WHEEL);
    shoot_storage_ctrl(KB_OPEN_BALL_STOR, KB_CLOSE_BALL_STOR);
    shoot_cmd_ctrl(KB_SINGLE_SHOOT, KB_CONTINUE_SHOOT);
  }
}

void keyboard_gimbal_hook(void)
{
  if (km.kb_enable)
  {
    gimbal_speed_ctrl(rc.mouse.y, rc.mouse.x);
  }
  else
  {
    km.pit_v = 0;
    km.yaw_v = 0;
    gimbal.track_ctrl = 0;
  }
}
