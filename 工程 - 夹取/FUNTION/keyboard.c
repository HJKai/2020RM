#include "keyboard.h"
#include "remote_ctrl.h"
#include "STM32_TIM_BASE.h"
#include "sys_config.h"
#include "ramp.h"
#include "clamp_task.h"
#include "rescue_task.h"
#include "supply_task.h"
#include "upraise_task.h"
#include "modeswitch_task.h"

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
          if(HAL_GetTick() - km.l_cnt > 500)
            *status = MOUSE_LONG;
        }
        else
        {
          if(HAL_GetTick() - km.r_cnt > 500)
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
  if (rc.sw1 == RC_MI && rc.sw2 != RC_DN)
    km.kb_enable = 1;
  else
    km.kb_enable = 0;
  
  if (km.kb_enable)
  {
    get_mouse_status(&km.l_mouse_sta, rc.mouse.l);
    get_mouse_status(&km.r_mouse_sta, rc.mouse.r);
  }
}


/*控制救援*/
static void rescue_ctrl(uint8_t rescue_open,uint8_t rescue_close)
{
  if(rescue_open) //抓住
  {
    rescue.rescue_cmd = 1;
  }
  if(rescue_close) //松开
  {
    rescue.rescue_cmd = 0;
  }
}

static void clamp_mode_ctrl(uint8_t clamp_big,uint8_t clamp_small)
{
  if(clamp_big)
  {
    clamp.island_ctrl = BIG_ISLAND;
  }
  if(clamp_small)
  {
    clamp.island_ctrl = SMALL_ISLAND;
  }
}

/*控制单个夹取和连续夹取命令*/
static void clamp_cmd_ctrl(uint8_t clamp_cmd,uint8_t c_clamp_cmd)
{
  if(clamp_cmd)
  {
    clamp.clamp_cmd = 1;
    clamp.c_clamp_cmd = 0;
  }
  if(c_clamp_cmd)
  {
//    clamp.clamp_cmd = 0;
    clamp.c_clamp_cmd = 1;
  }
  else
  {
    clamp.c_clamp_cmd = 0;
  } 
}

/*控制补给小弹丸*/
static void supply_ctrl(uint8_t supply_big,uint8_t supply_small,uint8_t supply_cmd)
{
  if(supply_big)
  {
    supply_mode = SUPPLY_TO_HERO;
  }
  
  if(supply_small)
  {
    supply_mode = SUPPLY_TO_INFANTRY;
  }
  
  if(supply_cmd)
  {
    supply.supply_cmd = 1;
  }
  else
  {
    supply.supply_cmd = 0;
  }
}


void keyboard_clamp_hook(void)
{
  if(km.kb_enable)
  {
    clamp_mode_ctrl(KB_CTRL_CLAMP_BIG,KB_CTRL_CLAMP_SMALL);
    clamp_cmd_ctrl(KB_SINGLE_CLAMP, KB_CONTINUE_CLAMP);
    if(KB_UPRAISE_INIT)
      upraise.state = INIT_NEVER;
  }
}

void keyboard_rescue_hook(void)
{
  if(km.kb_enable)
  {
    rescue_ctrl(KB_RESCUE_CTRL,KB_CLOSE_RESCUE_CTRL);
  }
}

void keyboard_supply_hook(void)
{
  if (km.kb_enable)
  {
    supply_ctrl(KB_SUPPLY_BIG, KB_SUPPLY_SMALL,KB_SUPPLY);
    if(KB_UPRAISE_INIT)
      upraise.state = INIT_NEVER;
  }
  
}
