/**
  ******************************************************************************
  * ������  �� ����STM32F4��׼�� 
  * оƬ�ͺţ� STM32F427IIH
  * ����汾�� ��һ�����
  * ������ڣ� 2019-11-9
  ******************************************************************************
  *                                RM 
  *
  *                  һ�걸����ãã��д���򣬵�������
  *                      ���д��룬Bug�δ��ء�
  *                  ��еÿ�����뷨������ģ�����æ��
  *
  *                  �Ӿ�����������������ģ�Ϧ�ϳ���
  *                      ������ԣ�Ω����ǧ�С�
  *                  ÿ��ƻ���ɺ����ҹ���£������Ρ�
  ******************************************************************************
**/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/*freertos*/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
/*config*/
#include "delay.h"
#include "gpio.h"
#include "can.h"
#include "usart.h"
#include "spi.h"
#include "tim.h"
#include "STM32_TIM_BASE.h"
/*bsp*/
#include "bsp_flash.h"
#include "judge_rx_data.h"
#include "judge_tx_data.h"
#include "pc_rx_data.h"
#include "pc_tx_data.h"
/*task*/
#include "start_task.h"
#include "detect_task.h"
#include "rescue_task.h"
#include "supply_task.h"
#include "clamp_task.h"
#include "upraise_task.h"
#include "stdlib.h"
/** @addtogroup Template_Project
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void flash_cali(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

int main(void)
{ 
	SysTick_Init(180);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	TIM_BASE_Init(10-1,9000-1);
	GPIO_INIT();
  TIM5_DEVICE(20000-1,90-1); 
	CAN1_DEVICE(CAN_Mode_Normal, CAN_SJW_1tq, CAN_BS1_3tq, CAN_BS2_5tq, 5);
	CAN2_DEVICE(CAN_Mode_Normal, CAN_SJW_4tq, CAN_BS1_3tq, CAN_BS2_5tq, 5);
	USART1_DEVICE();
  USART3_DEVICE();
  USART6_DEVICE();
  UART8_DEVICE();
	SPI_DEVICE();
	
	detect_param_init();
  clamp_param_init();
  rescue_param_init();
  supply_param_init();
  upraise_param_init();
  judgement_rx_param_init();
  judgement_tx_param_init();
  pc_rx_param_init();
  pc_tx_param_init();
	/*���ڲ�FLASH����У׼����*/
//	flash_cali();
  
	TASK_START();
	vTaskStartScheduler();
  /* Infinite loop */
  while (1)
  {
  }
}

void flash_cali(void)
{
  BSP_FLASH_READ();
  if(cali_param.cali_state != CALI_DONE)
  {
    for( ; ; );
  }
  else
  {

  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
