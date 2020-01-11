#include "judge_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "comm_task.h"
#include "detect_task.h"
#include "data_packet.h"
#include "judge_rx_data.h"
#include "judge_tx_data.h"

extern TaskHandle_t judge_rx_Task_Handle;

UBaseType_t judge_tx_stack_surplus;
UBaseType_t judge_rx_stack_surplus;

void judge_tx_task(void *parm)
{
	uint32_t judge_wake_time = osKernelSysTick();
  while(1)
  {
    judgement_client_packet_pack(&judge_send_mesg.client_custom_data);
		judgement_client_graphics_draw_pack();
    send_packed_fifo_data(&judge_txdata_fifo, DN_REG_ID);
    
    judge_tx_stack_surplus = uxTaskGetStackHighWaterMark(NULL);    
    vTaskDelayUntil(&judge_wake_time, 100);
  }
}

void judge_rx_task(void *parm)
{
  uint32_t Signal;
	BaseType_t STAUS;
  
  while(1)
  {
    STAUS = xTaskNotifyWait((uint32_t) NULL, 
                        (uint32_t) JUDGE_UART_IDLE_SIGNAL, 
                        (uint32_t *)&Signal, 
                        (TickType_t) portMAX_DELAY );
    if(STAUS == pdTRUE)
		{
			if(Signal & JUDGE_UART_IDLE_SIGNAL)
			{

        dma_buffer_to_unpack_buffer(&judge_rx_obj, UART_IDLE_IT);//通过指针取址的方法把串口接收的数据放进FIFO
        unpack_fifo_data(&judge_unpack_obj, DN_REG_ID);//同样再通过指针取址的方法把FIFO里的数据拿出来放进一个数组里
      }
    }
    judge_rx_stack_surplus = uxTaskGetStackHighWaterMark(NULL);    
  }
}




/*USART3 中断函数*/
void USART3_IRQHandler(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
	if(USART_GetFlagStatus(USART3,USART_FLAG_IDLE) != RESET 
		 && USART_GetITStatus(USART3,USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(USART3);
    USART_ClearFlag(USART3, USART_FLAG_IDLE);//清除空闲中断标志位
    
    err_detector_hook(JUDGE_SYS_OFFLINE);
    
    if(judge_rx_Task_Handle != NULL) //避免任务没来得及创建就发送信号量，导致卡在断言机制中
    {
      xTaskNotifyFromISR((TaskHandle_t) judge_rx_Task_Handle, 
                         (uint32_t) JUDGE_UART_IDLE_SIGNAL,
                         (eNotifyAction) eSetBits,
                         (BaseType_t *)&xHigherPriorityTaskWoken);
      /*进行上下文切换*/
      if(xHigherPriorityTaskWoken != pdFALSE)
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
	}
}


