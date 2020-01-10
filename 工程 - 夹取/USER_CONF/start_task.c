#include "start_task.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "comm_task.h"
#include "modeswitch_task.h"
#include "info_get_task.h"
#include "detect_task.h"
#include "judge_task.h"
#include "pc_task.h"

#include "clamp_task.h"
#include "supply_task.h"
#include "rescue_task.h"
#include "upraise_task.h"

#define START_TASK_SIZE 128
#define START_TASK_PRIO 2

#define CAN_MSG_SEND_TASK_SIZE 128
#define CAN_MSG_SEND_TASK_PRIO 6

#define CLAMP_TASK_SIZE 128
#define CLAMP_TASK_PRIO 5

#define SUPPLY_TASK_SIZE 128
#define SUPPLY_TASK_PRIO 5

#define RESCUE_TASK_SIZE 128
#define RESCUE_TASK_PRIO 5

#define UPRAISE_TASK_SIZE 128
#define UPRAISE_TASK_PRIO 5

#define MODE_SWITCH_TASK_SIZE 128
#define MODE_SWITCH_TASK_PRIO 4

#define INFO_GET_TASK_SIZE 128
#define INFO_GET_TASK_PRIO 4

#define DETECT_TASK_SIZE 128
#define DETECT_TASK_PRIO 4

#define JUDEG_TX_TASK_SIZE 256
#define JUDEG_TX_TASK_PRIO 4

#define JUDEG_RX_TASK_SIZE 256
#define JUDEG_RX_TASK_PRIO 4

#define PC_TX_TASK_SIZE 256
#define PC_TX_TASK_PRIO 4

#define PC_RX_TASK_SIZE 256
#define PC_RX_TASK_PRIO 4

TaskHandle_t start_Task_Handle;
TaskHandle_t clamp_Task_Handle;
TaskHandle_t supply_Task_Handle;
TaskHandle_t rescue_Task_Handle;
TaskHandle_t upraise_Task_Handle;
TaskHandle_t can_msg_send_Task_Handle;
TaskHandle_t mode_switch_Task_Handle;
TaskHandle_t info_get_Task_Handle;
TaskHandle_t detect_Task_Handle;

TaskHandle_t judge_tx_Task_Handle;
TaskHandle_t judge_rx_Task_Handle;
TaskHandle_t pc_tx_Task_Handle;
TaskHandle_t pc_rx_Task_Handle;

void start_task(void *parm)
{
	taskENTER_CRITICAL();
	
	{
    xTaskCreate((TaskFunction_t)clamp_task,
								(const char *)"clamp_task",
								(uint16_t)CLAMP_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)CLAMP_TASK_PRIO,
								(TaskHandle_t *) &clamp_Task_Handle );
                
    xTaskCreate((TaskFunction_t)supply_task,
								(const char *)"supply_task",
								(uint16_t)SUPPLY_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)SUPPLY_TASK_PRIO,
								(TaskHandle_t *) &supply_Task_Handle );
                
    xTaskCreate((TaskFunction_t)rescue_task,
								(const char *)"rescue_task",
								(uint16_t)RESCUE_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)RESCUE_TASK_PRIO,
								(TaskHandle_t *) &rescue_Task_Handle );
                
    xTaskCreate((TaskFunction_t)upraise_task,
								(const char *)"upraise_task",
								(uint16_t)UPRAISE_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)UPRAISE_TASK_PRIO,
								(TaskHandle_t *) &upraise_Task_Handle );
	
  }        

  {
		xTaskCreate((TaskFunction_t)can_msg_send_task,
								(const char *)"can_msg_send_task",
								(uint16_t)CAN_MSG_SEND_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)CAN_MSG_SEND_TASK_PRIO,
								(TaskHandle_t *) &can_msg_send_Task_Handle );
	}
	
	{						
		xTaskCreate((TaskFunction_t)mode_switch_task,
							(const char *)"mode_switch_task",
							(uint16_t)MODE_SWITCH_TASK_SIZE,
							(void * )NULL,
							(UBaseType_t)MODE_SWITCH_TASK_PRIO,
							(TaskHandle_t *) &mode_switch_Task_Handle );
							
		xTaskCreate((TaskFunction_t)info_get_task,
								(const char *)"info_get_task",
								(uint16_t)INFO_GET_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)INFO_GET_TASK_PRIO,
								(TaskHandle_t *) &info_get_Task_Handle );
								
		xTaskCreate((TaskFunction_t)detect_task,
								(const char *)"detect_task",
								(uint16_t)DETECT_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)DETECT_TASK_PRIO,
								(TaskHandle_t *) &detect_Task_Handle );
								
	}						
	
  {
    xTaskCreate((TaskFunction_t)judge_tx_task,
								(const char *)"judge_tx_task",
								(uint16_t)JUDEG_TX_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)JUDEG_TX_TASK_PRIO,
								(TaskHandle_t *) &judge_tx_Task_Handle );
                
    xTaskCreate((TaskFunction_t)judge_rx_task,
								(const char *)"judge_rx_task",
								(uint16_t)JUDEG_RX_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)JUDEG_RX_TASK_PRIO,
								(TaskHandle_t *) &judge_rx_Task_Handle );
                
    xTaskCreate((TaskFunction_t)pc_tx_task,
								(const char *)"pc_tx_task",
								(uint16_t)PC_TX_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)PC_TX_TASK_PRIO,
								(TaskHandle_t *) &pc_tx_Task_Handle );
                
    xTaskCreate((TaskFunction_t)pc_rx_task,
								(const char *)"pc_rx_task",
								(uint16_t)PC_RX_TASK_SIZE,
								(void * )NULL,
								(UBaseType_t)PC_RX_TASK_PRIO,
								(TaskHandle_t *) &pc_rx_Task_Handle );
    
  }
  
	vTaskDelete(start_Task_Handle);
	taskEXIT_CRITICAL();						
}

void TASK_START(void)
{
	xTaskCreate((TaskFunction_t)start_task,
							(const char *)"start_task",
							(uint16_t)START_TASK_SIZE,
							(void * )NULL,
							(UBaseType_t)START_TASK_PRIO,
							(TaskHandle_t *) &start_Task_Handle );
              
}

