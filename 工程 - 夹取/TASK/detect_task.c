#include "detect_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modeswitch_task.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

RCC_ClocksTypeDef RCC_Clocks;

UBaseType_t detect_stack_surplus;

global_err_t global_err;

State_t detect_state,detect_last_state;
char txt[24][2][20] = {{"",""},//0
                    {"������������","���������쳣"},//1
                    {"������������","���������쳣"},//2
                    {"������������","���������쳣"},//3
                    {"������������","���������쳣"},//4
                    {"yaw������","yaw���쳣"},//5
                    {"pitch������","pitch���쳣"},//6
                    {"��������","�����쳣"},//7
                    {"��Ħ��������","��Ħ�����쳣"},//8
                    {"��Ħ��������","��Ħ�����쳣"},//9
                    {"ң������","ң���쳣"},//10
                    {"����ϵͳ����","����ϵͳ�쳣"},//11
                    {"С��������","С�����쳣"},//12
                    {"��ȡ������","��ȡ���쳣"},//13
                    {"��ȡ������","��ȡ���쳣"},//14
                    {"��ȡ�ƶ�����","��ȡ�ƶ��쳣"},//15
                    {"̧��������","̧�����쳣"},//16
                    {"̧��������","̧�����쳣"},//17
                    {"��Ԯ������","��Ԯ���쳣"},//18
                    {"��Ԯ������","��Ԯ���쳣"},//19
                    {"��Ԯ������","��Ԯ���쳣"},//20
                    {"mcuͨ������","mcuͨ���쳣"},//21
                    {"����ϵͳ����","����ϵͳ�쳣"},//22
                    {"С��������","С�����쳣"}};//23
uint32_t temp1,temp2;
void detect_task(void *parm)
{
  char message[100];
  char *point = NULL;
	uint32_t detect_wake_time = osKernelSysTick();
  while(1)
  {
    module_offline_detect();//��ȡ��ȡ���ư����������
    detect_state.offline = global_err.offline | global_err.gimbal_offline;
    if(detect_state.offline != detect_last_state.offline) //�ж���������״̬�Ƿ����仯
    {
      taskENTER_CRITICAL();
      temp2 = detect_state.offline;
      temp1 = (detect_state.offline ^ detect_last_state.offline) >> 1;//��ȡ�����仯��λ Ϊ1˵���仯��0�򲻱�
      for(uint8_t id = CHASSIS_M1_OFFLINE; id <= PC_SYS_OFFLINE; id++)
      {
        if(temp1&0x00000001)//˵����λ�����˱仯
        {
          if((temp2 >> id)&0x00000001)//�жϸ�λ��ǰ��״̬ 
          {
            global_err.list[id].err_exist = 1;
            
            sprintf(message,"page%d.t%d.pco=RED",(id/13)+1,(id-1)%12); //��ɫ
            point = message;
            for(int i = 0;i < strlen((char*)message);i++ )
            {
              while((UART8->SR & USART_FLAG_TXE) != USART_FLAG_TXE)
              {
              }
              UART8->DR = (*point++ & (uint8_t)0xff);
            }
            for(int i = 0;i < 3;i++ )
            {
              for(int j = 0;j < sizeof(uint8_t);j++ )
              {
                while((UART8->SR & USART_FLAG_TXE) != USART_FLAG_TXE)
                {
                }
                UART8->DR = (uint8_t)0xff;
              }
            }
            sprintf(message,"page%d.t%d.txt=\"%s\"",(id/13)+1,(id-1)%12,txt[id][1]); //����
            point = message;
            for(int i = 0;i < strlen((char*)message);i++ )
            {
              while((UART8->SR & USART_FLAG_TXE) != USART_FLAG_TXE)
              {
              }
              UART8->DR = (*point++ & (uint8_t)0xff);
            }
            for(int i = 0;i < 3;i++ )
            {
              for(int j = 0;j < sizeof(uint8_t);j++ )
              {
                while((UART8->SR & USART_FLAG_TXE) != USART_FLAG_TXE)
                {
                }
                UART8->DR = (uint8_t)0xff;
              }
            }
          }
          else
          {
            global_err.list[id].err_exist = 0;
            
            sprintf(message,"page%d.t%d.pco=GREEN",(id/13)+1,(id-1)%12); //��ɫ
            point = message;
            for(int i = 0;i < strlen((char*)message);i++ )
            {
              while((UART8->SR & USART_FLAG_TXE) != USART_FLAG_TXE)
              {
              }
              UART8->DR = (*point++ & (uint8_t)0xff);
            }
            for(int i = 0;i < 3;i++ )
            {
              for(int j = 0;j < sizeof(uint8_t);j++ )
              {
                while((UART8->SR & USART_FLAG_TXE) != USART_FLAG_TXE)
                {
                }
                UART8->DR = (uint8_t)0xff;
              }
            }
            sprintf(message,"page%d.t%d.txt=\"%s\"",(id/13)+1,(id-1)%12,txt[id][0]); //����
            point = message;
            for(int i = 0;i < strlen((char*)message);i++ )
            {
              while((UART8->SR & USART_FLAG_TXE) != USART_FLAG_TXE)
              {
              }
              UART8->DR = (*point++ & (uint8_t)0xff);
            }
            for(int i = 0;i < 3;i++ )
            {
              for(int j = 0;j < sizeof(uint8_t);j++ )
              {
                while((UART8->SR & USART_FLAG_TXE) != USART_FLAG_TXE)
                {
                }
                UART8->DR = (uint8_t)0xff;
              }
            }
          }
        }
        else
        {
          temp1 = temp1 >> 1;
        }
      }
      detect_last_state.offline = detect_state.offline;//��ȡ��һ������״̬
      taskEXIT_CRITICAL();
    }
    
    RCC_GetClocksFreq(&RCC_Clocks);//����ⲿ�����Ƿ�����
    
    
    
    
    /**************debug****************/
    
//    sprintf(message,"page%d.t%d.pco=RED",1,1); //��ɫ
//    sprintf(message,"page%d.t%d.txt=\"%s\"",1,2,"����"); //����
//    point = message;
//    for(int i = 0;i <= strlen(message);i++ )
//    {
//        while((UART8->SR & USART_FLAG_TXE) != USART_FLAG_TXE)
//        {
//        }
//        UART8->DR = (*point++ & (uint8_t)0xFF);
//    }
    
    /*****************debug*********************/
    
    detect_stack_surplus = uxTaskGetStackHighWaterMark(NULL);    
    vTaskDelayUntil(&detect_wake_time, 50);
  }
}


void detect_param_init(void)
{
  for(uint8_t id = CLAMP_M1_OFFLINE; id <= PC_SYS_OFFLINE; id++)
  {
    global_err.list[id].param.set_timeout = 500;
    global_err.list[id].param.last_times = 0;
    global_err.list[id].param.delta_times = 0;
    global_err.list[id].err_exist = 0;
    global_err.err_now_id[id] = BOTTOM_DEVICE;
  }
  global_err.list[PC_SYS_OFFLINE].param.set_timeout = 2000;
  global_err.list[PC_SYS_OFFLINE].param.last_times = 0;
  global_err.list[PC_SYS_OFFLINE].param.delta_times = 0;
  global_err.list[PC_SYS_OFFLINE].err_exist = 0;
  global_err.err_now_id[PC_SYS_OFFLINE] = BOTTOM_DEVICE;
}

void err_detector_hook(int err_id)
{
  global_err.list[err_id].param.last_times = HAL_GetTick();
}

void module_offline_detect(void)
{ 
  for (uint8_t id = CLAMP_M1_OFFLINE; id <= PC_SYS_OFFLINE; id++)
  {
    global_err.list[id].param.delta_times = HAL_GetTick() - global_err.list[id].param.last_times;
    if(global_err.list[id].param.delta_times > global_err.list[id].param.set_timeout)
    {
      global_err.err_now_id[id] = (err_id)id;
      global_err.list[id].err_exist = 1;
      Set_bit(global_err.offline,id);
    }
    else
    {
      global_err.err_now_id[id] = BOTTOM_DEVICE;
      global_err.list[id].err_exist = 0;
      Reset_bit(global_err.offline,id);
    }
  }
}


uint8_t clamp_is_controllable(void)
{
  if (global_mode != GLOBAL_CLAMP_MODE
   || global_err.list[MCU_COMM_OFFLINE].err_exist
   || global_err.list[REMOTE_CTRL_OFFLINE1].err_exist
   || global_err.list[UPRAISE_M1_OFFLINE].err_exist
   || global_err.list[UPRAISE_M2_OFFLINE].err_exist)
    return 0;
  else
    return 1;
}

uint8_t rescue_is_controllable(void)
{
  if (global_mode != GLOBAL_RESCUE_MODE 
   || global_err.list[MCU_COMM_OFFLINE].err_exist
   || global_err.list[REMOTE_CTRL_OFFLINE1].err_exist
   || global_err.list[RESCUE_M1_OFFLINE].err_exist
   || global_err.list[RESCUE_M2_OFFLINE].err_exist
   || global_err.list[RESCUE_M3_OFFLINE].err_exist)
    return 0;
  else
    return 1;
}

uint8_t supply_is_controllable(void)
{
  if (global_mode != GLOBAL_SUPPLY_MODE 
   || global_err.list[MCU_COMM_OFFLINE].err_exist
   || global_err.list[REMOTE_CTRL_OFFLINE1].err_exist
   || global_err.list[UPRAISE_M1_OFFLINE].err_exist
   || global_err.list[UPRAISE_M2_OFFLINE].err_exist)
    return 0;
  else
    return 1;
}

/*UART8 �жϺ���*/
void UART8_IRQHandler(void)
{
	if(USART_GetFlagStatus(UART8,USART_FLAG_IDLE) != RESET 
		 && USART_GetITStatus(UART8,USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(UART8);		
		USART_ClearFlag(UART8, USART_FLAG_IDLE);//��������жϱ�־λ
    
	}
}

