#include "detect_task.h"
#include "STM32_TIM_BASE.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modeswitch_task.h"
#include "stdio.h"
#include "stdlib.h"

RCC_ClocksTypeDef RCC_Clocks;

UBaseType_t detect_stack_surplus;

global_err_t global_err;

State_t state,last_state;
char txt[24][2][20] = {{"",""},//0
                    {"������������","���������쳣"},//1
                    {"������������","���������쳣"},//2
                    {"������������","���������쳣"},//3
                    {"������������","���������쳣"},//4
                    {"��̨yaw������","��̨yaw���쳣"},//5
                    {"��̨pitch������","��̨pitch���쳣"},//6
                    {"��̨��������","��̨�����쳣"},//7
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
uint32_t temp1,temp2,timeout = 0;
void detect_task(void *parm)
{
//  uint32_t temp1,temp2,timeout = 0;
  char message[40];
  char *point = NULL;
	uint32_t detect_wake_time = osKernelSysTick();
  while(1)
  {
    module_offline_detect();//��ȡ��ȡ���ư����������
    state.offline = global_err.offline | global_err.gimbal_offline;
    if(state.offline != last_state.offline) //�ж���������״̬�Ƿ����仯
    {
      temp1 = state.offline ^ last_state.offline;//��ȡ�����仯��λ Ϊ1˵���仯��0�򲻱�
      temp2 = state.offline;
      for(uint8_t id = BOTTOM_DEVICE; id <= PC_SYS_OFFLINE; id++)
      {
        if(temp1&0x01)//˵����λ�����˱仯
        {
          if((temp2 >> id)&0x01)//�жϸ�λ��ǰ��״̬ 
          {
            global_err.list[id].err_exist = 1;
            
            sprintf(message,"page%d.t%d.pco=RED",(id/8)+1,id); //��ɫ
            sprintf(message,"page%d.t%d.txt=\"%s\"",(id/8)+1,id,txt[id][1]); //����
            point = message;
            for(int i = 0;i <= strlen(message);i++ )
            {
              UART8->DR = *point++;
              timeout = 500; //��ʱ
              while(((UART8->SR & USART_FLAG_TC) != (uint16_t)RESET) && timeout-- );
              UART8->SR = (uint16_t)~USART_FLAG_TC;
            }
            for(int i = 0;i < 3;i++ )
            {
              UART8->DR = 0XFF;
              timeout = 500; //��ʱ
              while(((UART8->SR & USART_FLAG_TC) != (uint16_t)RESET) && timeout-- );
              UART8->SR = (uint16_t)~USART_FLAG_TC;
            }
          }
          else
          {
            global_err.list[id].err_exist = 0;
            
            sprintf(message,"page%d.t%d.pco=GREEN",(id/8)+1,id); //��ɫ
            sprintf(message,"page%d.t%d.txt=\"%s\"",(id/8)+1,id,txt[id][0]); //����
            point = message;
            for(int i = 0;i <= strlen(message);i++ )
            {
              UART8->DR = *point++;
              timeout = 500; //��ʱ
              while(((UART8->SR & USART_FLAG_TC) != (uint16_t)RESET) && timeout--);
              UART8->SR = (uint16_t)~USART_FLAG_TC;
            }
            for(int i = 0;i < 3;i++ )
            {
              UART8->DR = 0XFF;
              timeout = 500; //��ʱ
              while(((UART8->SR & USART_FLAG_TC) != (uint16_t)RESET) && timeout-- );
              UART8->SR = (uint16_t)~USART_FLAG_TC;
            }
          }
        }
        else
        {
          temp1 = temp1 >> 1;
        }
      }
    }
    
    RCC_GetClocksFreq(&RCC_Clocks);//����ⲿ�����Ƿ�����
    
    last_state = state;//��ȡ��һ������״̬
    
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
/*   || global_err.list[UPRAISE_M1_OFFLINE].err_exist
   || global_err.list[UPRAISE_M2_OFFLINE].err_exist*/)
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
