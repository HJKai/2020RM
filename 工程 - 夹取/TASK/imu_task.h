#ifndef __IMU_TASK_H__
#define __IMU_TASK_H__

#include "stm32f4xx.h"

/* imu task period time (ms) */
#define IMU_TASK_PERIOD 1



void imu_task(void *parm);


#endif
