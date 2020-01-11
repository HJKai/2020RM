#include "imu_task.h"
#include "arm_math.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "gimbal_task.h"
#include "bsp_can.h"

UBaseType_t imu_stack_surplus;



void imu_task(void *parm)
{
	uint32_t imu_wake_time = osKernelSysTick();
  while(1)
  {
//    gimbal.sensor.yaw_gyro_angle = mpu_data.yaw_angle;
    
    imu_stack_surplus = uxTaskGetStackHighWaterMark(NULL);
    
    vTaskDelayUntil(&imu_wake_time, IMU_TASK_PERIOD);
  }
}

