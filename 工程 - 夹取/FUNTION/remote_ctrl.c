#include "remote_ctrl.h"
#include "stdlib.h"
#include "string.h"
#include "sys_config.h"
#include "STM32_TIM_BASE.h"
#include "keyboard.h"

#include "clamp_task.h"
#include "rescue_task.h"
#include "supply_task.h"
#include "upraise_task.h"
#include "modeswitch_task.h"

rc_ctrl_t rm;
sw_record_t glb_sw;

/*补给*/
static void supply_operation_func(uint8_t supply_big, uint8_t supply_small, uint8_t supply_cmd)
{
  if (supply_big)
  {
    supply_mode = SUPPLY_TO_HERO;
  }
	if (supply_small)
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

static void rc_rescue_cmd(uint8_t rescue_ctrl)
{
  if(rescue_ctrl)
  {
    rescue.rescue_cmd = !rescue.rescue_cmd;
  }
}

//static void rc_clamp_mode(uint8_t clamp_big, uint8_t clamp_small)
//{
//  if(clamp_big)  //大资源岛
//  {
//    clamp.island_ctrl = BIG_ISLAND;
//  }
//  if(clamp_small) //小资源岛
//  {
//    clamp.island_ctrl = SMALL_ISLAND;
//  }
//}

/*夹单个和夹多个*/
static void rc_clamp_cmd(uint8_t small_single_clamp, uint8_t small_continue_clamp, uint8_t big_single_clamp, uint8_t big_continue_clamp)
{ 
  if (small_single_clamp || big_single_clamp)
  {
    clamp.c_clamp_time = HAL_GetTick();
    clamp.clamp_cmd = 1;
    clamp.c_clamp_cmd = 0;
  }
  else if ((small_continue_clamp || big_continue_clamp) && (HAL_GetTick() - clamp.c_clamp_time >= 500))	//单发和连发相隔150毫秒
  {
//    clamp.clamp_cmd = 0;
    clamp.c_clamp_cmd = 1;
  }
  else
  { 
    clamp.c_clamp_cmd = 0;
  }
  
  if(small_single_clamp || small_continue_clamp)
  {
    clamp.island_ctrl = SMALL_ISLAND;
  }
  if(big_single_clamp || big_continue_clamp)
  {
    clamp.island_ctrl = BIG_ISLAND;
  }
}

void remote_ctrl_clamp_hook(void)
{
//  //夹取模式
//  rc_clamp_mode(RC_CLAMP_BIG, RC_CLAMP_SMALL);
	//夹取使能
  rc_clamp_cmd(RC_SMALL_SINGLE_CLAMP, RC_SMALL_CONTINUE_CLAMP, RC_BIG_SINGLE_CLAMP, RC_BIG_CONTINUE_CLAMP);
  if(RC_UPRAISE_INIT)
  {
    upraise.state = INIT_NEVER;
  }
}

void remote_ctrl_rescue_hook(void)
{
  rc_rescue_cmd(RC_RESCUE_MODE);
}

void remote_ctrl_supply_hook(void)
{
  supply_operation_func(RC_SUPPLY_BIG, RC_SUPPLY_SMALL, RC_SUPPLY_CMD);
  if(RC_UPRAISE_INIT)
  {
    upraise.state = INIT_NEVER;
  }
}
