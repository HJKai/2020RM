#include "bsp_can.h"
#include "detect_task.h"
#include "sys_config.h"
#include "modeswitch_task.h"
#include "stdlib.h"
#include "rc.h"

CanRxMsg rx1_message;
CanRxMsg rx2_message;

moto_measure_t moto_clamp[3]; //夹取移动   3508
moto_measure_t moto_upraise[2]; //抬升机构 3508
moto_measure_t moto_rescue[3]; //救援机构  2006


static void STD_CAN_RxCpltCallback(CAN_TypeDef *_hcan,CanRxMsg *message)
{
	if(_hcan == CAN1)
	{
		switch(message->StdId)
		{
			case CAN_CLAMP_M1_ID:
			case CAN_CLAMP_M2_ID:
			case CAN_CLAMP_M3_ID:
			{
				static uint8_t i = 0;
        //处理电机ID号
        i = message->StdId - CAN_CLAMP_M1_ID;
        //处理电机数据宏函数
        moto_clamp[i].msg_cnt++ <= 50 ? get_moto_offset(&moto_clamp[i], message) : encoder_data_handler(&moto_clamp[i], message);
        //记录时间
        err_detector_hook(CLAMP_M1_OFFLINE + i);
       
			}break;
			
			case CAN_RESCUE_M1_ID:
      case CAN_RESCUE_M2_ID:
      case CAN_RESCUE_M3_ID:
			{
				static uint8_t i = 0;
        //处理电机ID号
        i = message->StdId - CAN_RESCUE_M1_ID;
        //处理电机数据宏函数
        moto_rescue[i].msg_cnt++ <= 50 ? get_moto_offset(&moto_rescue[i], message) : encoder_data_handler(&moto_rescue[i], message);
        //记录时间
        err_detector_hook(RESCUE_M1_OFFLINE + i);  
			}break;
      
			default:
			{
			}break;
		}
	}
	else
	{
		switch(message->StdId)
    {
      case CAN_UPRAISE_M1_ID:
      case CAN_UPRAISE_M2_ID:
      {
        static uint8_t i = 0;
        //处理电机ID号
        i = message->StdId - CAN_UPRAISE_M1_ID;
        //处理电机数据宏函数
        moto_upraise[i].msg_cnt++ <= 50 ? get_moto_offset(&moto_upraise[i], message) : encoder_data_handler(&moto_upraise[i], message);
        err_detector_hook(UPRAISE_M1_OFFLINE + i);
      }break;
      /*遥控ID*/
      case 0x301:
      {
        rc.ch1 = (int16_t)(rx2_message.Data[0] << 8 | rx2_message.Data[1]);
        rc.ch2 = (int16_t)(rx2_message.Data[2] << 8 | rx2_message.Data[3]);
        rc.ch3 = (int16_t)(rx2_message.Data[4] << 8 | rx2_message.Data[5]);
        rc.ch4 = (int16_t)(rx2_message.Data[6] << 8 | rx2_message.Data[7]);
        err_detector_hook(MCU_COMM_OFFLINE);
      }break;
      
      case 0x302:
      {
        rc.sw1 = rx2_message.Data[0];
        rc.sw2 = rx2_message.Data[1];
        rc.iw = (int16_t)(rx2_message.Data[2] << 8 | rx2_message.Data[3]);
        rc.mouse.l = rx2_message.Data[4];
        rc.mouse.r = rx2_message.Data[5];
        global_mode = (global_status)rx2_message.Data[6]; //获取总的状态
        err_detector_hook(MCU_COMM_OFFLINE);
      }break;
      
      case 0x303:
      {
        rc.mouse.x = (int16_t)(rx2_message.Data[0] << 8 | rx2_message.Data[1]);
        rc.mouse.y = (int16_t)(rx2_message.Data[2] << 8 | rx2_message.Data[3]);
        rc.mouse.z = (int16_t)(rx2_message.Data[4] << 8 | rx2_message.Data[5]);
        rc.kb.key_code = (int16_t)(rx2_message.Data[6] << 8 | rx2_message.Data[7]);
        err_detector_hook(MCU_COMM_OFFLINE);
      }break;
      
      case 0x304:
      {
        global_err.gimbal_offline = (uint32_t)(rx2_message.Data[3] << 24 | rx2_message.Data[2] << 16 | rx2_message.Data[1] << 8 | rx2_message.Data[0]);
        err_detector_hook(MCU_COMM_OFFLINE);
      }break;
      
      default:
      {
      }break;
    }
	}
}


void encoder_data_handler(moto_measure_t* ptr, CanRxMsg *message)
{
  ptr->last_ecd = ptr->ecd;
  ptr->ecd      = (uint16_t)(message->Data[0] << 8 | message->Data[1]);
  
  if (ptr->ecd - ptr->last_ecd > 4096)
  {
    ptr->round_cnt--;
    ptr->ecd_raw_rate = ptr->ecd - ptr->last_ecd - 8192;
  }
  else if (ptr->ecd - ptr->last_ecd < -4096)
  {
    ptr->round_cnt++;
    ptr->ecd_raw_rate = ptr->ecd - ptr->last_ecd + 8192;
  }
  else
  {
    ptr->ecd_raw_rate = ptr->ecd - ptr->last_ecd;
  }

  ptr->total_ecd = ptr->round_cnt * 8192 + ptr->ecd - ptr->offset_ecd;
  /* 计算总角度 */
	if(ptr == &moto_rescue[0] || ptr == &moto_rescue[1] || ptr == &moto_rescue[2]) 
		ptr->total_angle = ptr->total_ecd / (ENCODER_ANGLE_RATIO*DECELE_RATIO_2006);//2006
	else
		ptr->total_angle = ptr->total_ecd / (ENCODER_ANGLE_RATIO*DECELE_RATIO_3508);//3508
	
	ptr->speed_rpm     = (int16_t)(message->Data[2] << 8 | message->Data[3]);
  ptr->given_current = (int16_t)(message->Data[4] << 8 | message->Data[5]);

}


/**
  * @brief     get motor initialize offset value
  * @param     ptr: Pointer to a moto_measure_t structure
  * @retval    None
  * @attention this function should be called after system can init
  */
void get_moto_offset(moto_measure_t* ptr, CanRxMsg *message)
{
    ptr->ecd        = (uint16_t)(message->Data[0] << 8 | message->Data[1]);
    ptr->offset_ecd = ptr->ecd;
}


/*发送夹取电机电流*/
void send_clamp_cur(int16_t iq1, int16_t iq2, int16_t iq3)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = CAN_CLAMP_ALL_ID;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = iq1 >> 8;
    TxMessage.Data[1] = iq1;
    TxMessage.Data[2] = iq2 >> 8;
    TxMessage.Data[3] = iq2;
    TxMessage.Data[4] = iq3 >> 8;
    TxMessage.Data[5] = iq3;
    TxMessage.Data[6] = 0;
    TxMessage.Data[7] = 0;

    CAN_Transmit(CLAMP_CAN, &TxMessage);
}

/*发送抬升电机电流*/
void send_upraise_cur(int16_t iq1, int16_t iq2)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = CAN_UPRAISE_ALL_ID;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = iq1 >> 8;
    TxMessage.Data[1] = iq1;
    TxMessage.Data[2] = iq2 >> 8;
    TxMessage.Data[3] = iq2;
    TxMessage.Data[4] = 0;
    TxMessage.Data[5] = 0;
    TxMessage.Data[6] = 0;
    TxMessage.Data[7] = 0;

    CAN_Transmit(UPRAISE_CAN, &TxMessage);
}

/*发送救援电机电流*/
void send_rescue_cur(int16_t iq1, int16_t iq2, int16_t iq3)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = CAN_RESCUE_ALL_ID;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = iq1 >> 8;
    TxMessage.Data[1] = iq1;
    TxMessage.Data[2] = iq2 >> 8;
    TxMessage.Data[3] = iq2;
    TxMessage.Data[4] = 0;
    TxMessage.Data[5] = 0;
    TxMessage.Data[6] = 0;
    TxMessage.Data[7] = 0;

    CAN_Transmit(RESCUE_CAN, &TxMessage);
}

//can1中断
void CAN1_RX0_IRQHandler(void)
{
    if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
    {
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
        CAN_Receive(CAN1, CAN_FIFO0, &rx1_message);
        STD_CAN_RxCpltCallback(CAN1,&rx1_message);
    }
}

//can2中断
void CAN2_RX0_IRQHandler(void)
{
    if (CAN_GetITStatus(CAN2, CAN_IT_FMP0) != RESET)
    {
        CAN_ClearITPendingBit(CAN2, CAN_IT_FMP0);
        CAN_Receive(CAN2, CAN_FIFO0, &rx2_message);
        STD_CAN_RxCpltCallback(CAN2,&rx2_message);
    }
}


