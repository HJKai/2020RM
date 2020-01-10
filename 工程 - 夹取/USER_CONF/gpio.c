#include "gpio.h"
#include "delay.h"
#include "bsp_flash.h"
#include "bsp_can.h"

static void KEY_EXTI(void);

/**************************************************
GPIOB:PIN2  key����
***************************************************/
void GPIO_INIT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI,ENABLE);
	
	/*GPIOA*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
  
	GPIO_SetBits(GPIOA,GPIO_Pin_4|GPIO_Pin_5);
	
	/*GPIOB*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
  
  GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
	
  /*GPIOC*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
  
  GPIO_SetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);
  
  /*GPIOE*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_12;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
  
  GPIO_SetBits(GPIOE,GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_12);
  
  /*GPIOF*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10;
	GPIO_Init(GPIOF,&GPIO_InitStructure);
  
  GPIO_SetBits(GPIOF,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10);
  
  /*GPIOI*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOI,&GPIO_InitStructure);
  
  GPIO_SetBits(GPIOI,GPIO_Pin_9);
  
	/*KEY����*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	KEY_EXTI();
}


/*****************************************************************
PB2:KEY�����ⲿ�ⲿ�ж�����
******************************************************************/
static void KEY_EXTI(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);			//ʹ���ⲿ�ж�Դʱ��
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB,EXTI_PinSource2);	//ӳ�䵽IO��
	
	//EXTI2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;//EXTI2�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
	EXTI_InitStructure.EXTI_Line=EXTI_Line2; 
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}


uint8_t key_state;//����״̬��־λ

void EXTI2_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line2)==1)
	{
		delay_ms(10);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2)==Bit_SET)
		{	
			
//			cali_param.yaw_offset = moto_clamp[0].ecd;
//			cali_param.pitch_offset = moto_clamp[1].ecd;
      cali_param.cali_state = CALI_DONE;
			key_state = BSP_FLASH_WRITE((uint8_t *)&cali_param,sizeof(cali_sys_t));
			
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line2);    
}
