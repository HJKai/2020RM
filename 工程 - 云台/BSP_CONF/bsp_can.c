#include "bsp_can.h"
#include "detect_task.h"
#include "sys_config.h"
#include "gimbal_task.h"
#include "stdlib.h"
#include "rc.h"
#include "delay.h"
#include "modeswitch_task.h"

CanRxMsg rx1_message;
CanRxMsg rx2_message;

moto_measure_t moto_chassis[4];
moto_measure_t moto_pit;
moto_measure_t moto_yaw;
moto_measure_t moto_trigger;
moto_measure_t moto_fric[2];
mpu_data_t mpu_data;

float angle;
float *p_angle = &angle;

float *yaw;
int16_t *gz,*gy;
static void STD_CAN_RxCpltCallback(CAN_TypeDef *_hcan,CanRxMsg *message)
{
	if(_hcan == CAN1)
	{
		switch(message->StdId)
		{
			case CAN_3508_M1_ID:
			case CAN_3508_M2_ID:
			case CAN_3508_M3_ID:
			case CAN_3508_M4_ID:
			{
				static uint8_t i = 0;
        //处理电机ID号
        i = message->StdId - CAN_3508_M1_ID;
        //处理电机数据宏函数
        moto_chassis[i].msg_cnt++ <= 50 ? get_moto_offset(&moto_chassis[i], message) : encoder_data_handler(&moto_chassis[i], message);
        //记录时间
        err_detector_hook(CHASSIS_M1_OFFLINE + i);
       
			}break;
			
			case CAN_YAW_MOTOR_ID:
			{
				encoder_data_handler(&moto_yaw, message);
				err_detector_hook(GIMBAL_YAW_OFFLINE);
			}break;
			
			case CAN_PIT_MOTOR_ID:
			{
				encoder_data_handler(&moto_pit, message);
				err_detector_hook(GIMBAL_PIT_OFFLINE);
			}break;
			
			case CAN_TRIGGER_MOTOR_ID:
			{
				moto_trigger.msg_cnt++ <= 50 ? get_moto_offset(&moto_trigger, message) : encoder_data_handler(&moto_trigger, message);
        err_detector_hook(TRIGGER_MOTO_OFFLINE);  
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
//      case CAN_FRIC_M1_ID:
//      case CAN_FRIC_M2_ID:
//      {
//        static uint8_t i = 0;
//        //处理电机ID号
//        i = message->StdId - CAN_FRIC_M1_ID;
//        //处理电机数据宏函数
//        moto_fric[i].msg_cnt++ <= 50 ? get_moto_offset(&moto_fric[i], message) : encoder_data_handler(&moto_fric[i], message);
//        err_detector_hook(FRI_MOTO1_OFFLINE + i);
//      }break;
      
      case 0x401:
      {
        uint8_t *px = rx2_message.Data;
        for(int i = 0; i < 4; i++)
        {
          *((uint8_t*)p_angle + i) = *(px + i);
        }
        
        gimbal.sensor.yaw_gyro_angle = angle;
        gimbal.sensor.yaw_palstance = (int16_t)(rx2_message.Data[5] << 8 | rx2_message.Data[4]);
        gimbal.sensor.yaw_palstance /= 16.384f;
        gimbal.sensor.pit_palstance = (int16_t)(rx2_message.Data[7] << 8 | rx2_message.Data[6]);
        gimbal.sensor.pit_palstance /= 16.384f;
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
  /* total angle, unit is degree */
	if(ptr == &moto_trigger)
		ptr->total_angle = ptr->total_ecd / (ENCODER_ANGLE_RATIO*36);
	else
		ptr->total_angle = ptr->total_ecd / ENCODER_ANGLE_RATIO;
	
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


/**
  * @brief  send current which pid calculate to esc. message to calibrate 6025 gimbal motor esc
  * @param  current value corresponding motor(yaw/pitch/trigger)
  */
void send_gimbal_cur(int16_t yaw_iq, int16_t pit_iq, int16_t trigger_iq)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = CAN_GIMBAL_ALL_ID;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = yaw_iq >> 8;
    TxMessage.Data[1] = yaw_iq;
    TxMessage.Data[2] = pit_iq >> 8;
    TxMessage.Data[3] = pit_iq;
    TxMessage.Data[4] = trigger_iq >> 8;
    TxMessage.Data[5] = trigger_iq;
    TxMessage.Data[6] = 0;
    TxMessage.Data[7] = 0;

    CAN_Transmit(GIMBAL_CAN, &TxMessage);
}

void send_chassis_cur(int16_t iq1, int16_t iq2, int16_t iq3, int16_t iq4)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = CAN_CHASSIS_ALL_ID;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = iq1 >> 8;
    TxMessage.Data[1] = iq1;
    TxMessage.Data[2] = iq2 >> 8;
    TxMessage.Data[3] = iq2;
    TxMessage.Data[4] = iq3 >> 8;
    TxMessage.Data[5] = iq3;
    TxMessage.Data[6] = iq4 >> 8;
    TxMessage.Data[7] = iq4;

    CAN_Transmit(CHASSIS_CAN, &TxMessage);
}

void send_fric_cur(int16_t iq1, int16_t iq2)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = CAN_FRIC_ALL_ID;
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
    
    CAN_Transmit(FRIC_CAN, &TxMessage);
}

void send_rc_data1(void)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = 0x301;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = rc.ch1 >> 8;//高八位
    TxMessage.Data[1] = rc.ch1; //低八位
    TxMessage.Data[2] = rc.ch2 >> 8;
    TxMessage.Data[3] = rc.ch2;
    TxMessage.Data[4] = rc.ch3 >> 8;
    TxMessage.Data[5] = rc.ch3;
    TxMessage.Data[6] = rc.ch4 >> 8;
    TxMessage.Data[7] = rc.ch4;

    CAN_Transmit(RC_CAN, &TxMessage);
}

void send_rc_data2(void)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = 0x302;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = rc.sw1;
    TxMessage.Data[1] = rc.sw2;
    TxMessage.Data[2] = rc.iw >> 8;
    TxMessage.Data[3] = rc.iw;
    TxMessage.Data[4] = rc.mouse.l;
    TxMessage.Data[5] = rc.mouse.r;
    TxMessage.Data[6] = (uint8_t)gimbal_mode;
    TxMessage.Data[7] = 0;
    
    CAN_Transmit(RC_CAN, &TxMessage);
}

void send_rc_data3(void)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = 0x303;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = rc.mouse.x >> 8;
    TxMessage.Data[1] = rc.mouse.x;
    TxMessage.Data[2] = rc.mouse.y >> 8;
    TxMessage.Data[3] = rc.mouse.y;
    TxMessage.Data[4] = rc.mouse.z >> 8;
    TxMessage.Data[5] = rc.mouse.z;
    TxMessage.Data[6] = rc.kb.key_code >> 8;
    TxMessage.Data[7] = rc.kb.key_code;

    CAN_Transmit(RC_CAN, &TxMessage);
}

void send_detect_state(void)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = 0x304;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = global_err.offline;//低位
    TxMessage.Data[1] = global_err.offline >> 8;//高位
    TxMessage.Data[2] = 0;
    TxMessage.Data[3] = 0;
    TxMessage.Data[4] = 0;
    TxMessage.Data[5] = 0;
    TxMessage.Data[6] = 0;
    TxMessage.Data[7] = 0;
    
    CAN_Transmit(DETECT_CAN, &TxMessage);
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


