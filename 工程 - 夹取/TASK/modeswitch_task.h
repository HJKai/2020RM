#ifndef _modeswitch_task_H
#define _modeswitch_task_H


#include "stm32f4xx.h"
#include "string.h"

#define MEMSET(flag,type) (memset((type*)flag,0,sizeof(type)))

typedef enum
{
  INIT_NEVER,
  INIT_DONE,
}global_state;

/*主要控制模式*/
typedef enum
{
  GLOBAL_RELEASE = 0,
  GLOBAL_CLAMP_MODE = 3,
  GLOBAL_RESCUE_MODE = 4,
  GLOBAL_SUPPLY_MODE = 5,
}global_status;
/*补给控制模式*/
typedef enum
{
  SUPPLY_INIT = 0,
  SUPPLY_TO_HERO,  //给英雄补给
  SUPPLY_TO_INFANTRY,  //给步兵补给
}supply_status;

/*救援控制模式*/
typedef enum
{
  RESCUE_INIT = 0,
  RESCUE_ENABLE, //刷复活卡
}rescue_status;

/*夹取控制救援*/
typedef enum
{
  CLAMP_INIT = 0,
  SMALL_ISLAND,
  BIG_ISLAND, 
  AUTOMATIC_SMALL_ISLAND,
  AUTOMATIC_BIG_ISLAND,
}clamp_status;

extern global_status global_mode; 
extern global_status last_global_mode;

extern clamp_status clamp_mode;
extern clamp_status last_clamp_mode;

extern rescue_status rescue_mode;
extern rescue_status last_rescue_mode;

extern supply_status supply_mode;
extern supply_status last_supply_mode;

void mode_switch_task(void *parm);
void get_last_mode(void);
void get_main_mode(void);
void get_clamp_mode(void);
void get_rescue_mode(void);
void get_supply_mode(void);

#endif

