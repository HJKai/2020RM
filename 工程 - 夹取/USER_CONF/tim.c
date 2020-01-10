#include "tim.h"

void TIM5_DEVICE(int per,int psc)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
  
  GPIO_PinAFConfig(GPIOI,GPIO_PinSource0,GPIO_AF_TIM5);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(GPIOI,&GPIO_InitStructure);
  
  TIM_TimeBaseInitStructure.TIM_Period = per;
  TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
  TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);
  
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 1000;
  
  TIM_OC4Init(TIM5,&TIM_OCInitStructure);

  
  TIM_OC4PreloadConfig(TIM5,TIM_OCPreload_Enable);
  TIM_ARRPreloadConfig(TIM5,ENABLE);
  
  TIM_Cmd(TIM5,ENABLE);
}
