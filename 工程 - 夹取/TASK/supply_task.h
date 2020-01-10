#ifndef _supply_task_H
#define _supply_task_H


#include "stm32f4xx.h"

typedef struct
{
  
  uint8_t supply_cmd;
  
}supply_t;

extern supply_t supply;

void supply_task(void *parm);
void supply_param_init(void);
void supply_init_handler(void);
void supply_to_hero_handler(void);
void supply_to_infantry_handler(void);

#endif

