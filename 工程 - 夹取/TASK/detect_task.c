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
                    {"底盘右上正常","底盘右上异常"},//1
                    {"底盘左上正常","底盘左上异常"},//2
                    {"底盘左下正常","底盘左下异常"},//3
                    {"底盘右下正常","底盘右下异常"},//4
                    {"yaw轴正常","yaw轴异常"},//5
                    {"pitch轴正常","pitch轴异常"},//6
                    {"拨盘正常","拨盘异常"},//7
                    {"左摩擦轮正常","左摩擦轮异常"},//8
                    {"右摩擦轮正常","右摩擦轮异常"},//9
                    {"遥控正常","遥控异常"},//10
                    {"裁判系统正常","裁判系统异常"},//11
                    {"小电脑正常","小电脑异常"},//12
                    {"夹取左正常","夹取左异常"},//13
                    {"夹取右正常","夹取右异常"},//14
                    {"夹取移动正常","夹取移动异常"},//15
                    {"抬升左正常","抬升左异常"},//16
                    {"抬升右正常","抬升右异常"},//17
                    {"救援左正常","救援左异常"},//18
                    {"救援右正常","救援右异常"},//19
                    {"救援下正常","救援下异常"},//20
                    {"mcu通信正常","mcu通信异常"},//21
                    {"裁判系统正常","裁判系统异常"},//22
                    {"小电脑正常","小电脑异常"}};//23
uint32_t temp1,temp2;
void detect_task(void *parm)
{
  char message[100];
  char *point = NULL;
	uint32_t detect_wake_time = osKernelSysTick();
  while(1)
  {
    module_offline_detect();//获取夹取控制板的离线数据
    detect_state.offline = global_err.offline | global_err.gimbal_offline;
    if(detect_state.offline != detect_last_state.offline) //判断整体离线状态是否发生变化
    {
      taskENTER_CRITICAL();
      temp2 = detect_state.offline;
      temp1 = (detect_state.offline ^ detect_last_state.offline) >> 1;//获取发生变化的位 为1说明变化，0则不变
      for(uint8_t id = CHASSIS_M1_OFFLINE; id <= PC_SYS_OFFLINE; id++)
      {
        if(temp1&0x00000001)//说明该位发生了变化
        {
          if((temp2 >> id)&0x00000001)//判断该位当前的状态 
          {
            global_err.list[id].err_exist = 1;
            
            sprintf(message,"page%d.t%d.pco=RED",(id/13)+1,(id-1)%12); //红色
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
            sprintf(message,"page%d.t%d.txt=\"%s\"",(id/13)+1,(id-1)%12,txt[id][1]); //离线
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
            
            sprintf(message,"page%d.t%d.pco=GREEN",(id/13)+1,(id-1)%12); //绿色
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
            sprintf(message,"page%d.t%d.txt=\"%s\"",(id/13)+1,(id-1)%12,txt[id][0]); //在线
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
      detect_last_state.offline = detect_state.offline;//获取上一次离线状态
      taskEXIT_CRITICAL();
    }
    
    RCC_GetClocksFreq(&RCC_Clocks);//检测外部晶振是否起振
    
    
    
    
    /**************debug****************/
    
//    sprintf(message,"page%d.t%d.pco=RED",1,1); //红色
//    sprintf(message,"page%d.t%d.txt=\"%s\"",1,2,"离线"); //离线
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

/*UART8 中断函数*/
void UART8_IRQHandler(void)
{
	if(USART_GetFlagStatus(UART8,USART_FLAG_IDLE) != RESET 
		 && USART_GetITStatus(UART8,USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(UART8);		
		USART_ClearFlag(UART8, USART_FLAG_IDLE);//清除空闲中断标志位
    
	}
}

