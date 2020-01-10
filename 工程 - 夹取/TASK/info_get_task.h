#ifndef info_get_task_H
#define info_get_task_H



#include "stm32f4xx.h"


void info_get_task(void *parm);

static void get_clamp_info(void);
static void get_rescue_info(void);
static void get_upraise_info(void);
static void get_global_last_info(void);

#endif
