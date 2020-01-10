#include "judge_rx_data.h"
#include "dma.h"
#include "string.h"
#include "judge_task.h"

//judge_rxdata_t judge_recv_mesg;
receive_judge_t judge_recv_mesg;
/* judge system dma receive data object */
uart_dma_rxdata_t judge_rx_obj;
//----------�������
/* unpack object */
unpack_data_t judge_unpack_obj;
/*����ϵͳ����*/
static SemaphoreHandle_t judge_rxdata_mutex;
fifo_s_t  judge_rxdata_fifo;
static uint8_t   judge_rxdata_buf[JUDGE_RX_FIFO_BUFLEN];

void judgement_rx_param_init(void)
{
  /* create the judge_rxdata_mutex mutex  */  
  judge_rxdata_mutex = xSemaphoreCreateMutex();
    
  /* judge data fifo init */
  fifo_s_init(&judge_rxdata_fifo, judge_rxdata_buf, JUDGE_RX_FIFO_BUFLEN, judge_rxdata_mutex); //���judge_rxdata_fifo�Ļ�����

  
  /* initial judge data dma receiver object */
  judge_rx_obj.dma_stream = DMA1_Stream1;
  judge_rx_obj.data_fifo = &judge_rxdata_fifo;
  judge_rx_obj.buff_size = JUDGE_RX_FIFO_BUFLEN;
  judge_rx_obj.buff[0] = judge_rxbuf[0];
  judge_rx_obj.buff[1] = judge_rxbuf[1];

  /* initial judge data unpack object */
  judge_unpack_obj.data_fifo = &judge_rxdata_fifo;
  judge_unpack_obj.p_header = (frame_header_t *)judge_unpack_obj.protocol_packet;
  judge_unpack_obj.index = 0;
  judge_unpack_obj.data_len = 0;
  judge_unpack_obj.unpack_step = STEP_HEADER_SOF;
  
}



uint8_t interover_data_overflow;
/*һ֡���ݰ�����֡ͷHEADER + ����CMD + ����data + У��λCRC*/
/*@ p_frame�������׵�ַ*/
void judgement_data_handler(uint8_t *p_frame)
{
  frame_header_t *p_header = (frame_header_t*)p_frame;
  memcpy(p_header, p_frame, HEADER_LEN);
  
  uint16_t data_length = p_header->data_length;//���ݳ���
  uint16_t cmd_id      = *(uint16_t *)(p_frame + HEADER_LEN);//�����׵�ַ + ֡ͷ���� = CMD�Ŀ�ʼ��ַ
  uint8_t *data_addr   = p_frame + HEADER_LEN + CMD_LEN;  //�����׵�ַ + ֡ͷ���� + CMD���� = ���ݵĿ�ʼ��ַ
    
  switch (cmd_id)
  {
//    case GAME_STATE_ID:
//    {
//      memcpy(&judge_recv_mesg.game_state, data_addr, data_length);
//       if (judge_recv_mesg.game_state.game_progress == 3)  //5 seconds count down
//       {
//         /*�û�����*/
//       }
//      
//      if (judge_recv_mesg.game_state.game_progress == 1)   //prepare stage
//      {
//        if (judge_recv_mesg.game_state.stage_remain_time < 240)
//        {
//          /*�û�����*/
//        }
//      }
//      else
//      {
//          /*�û�����*/
//      }
//    }
//    break;

//	  case GAME_RESULT_ID:
//      memcpy(&judge_recv_mesg.game_result, data_addr, data_length);
//    break;

//    case ROBOT_SURVIVORS_ID:
//      memcpy(&judge_recv_mesg.game_robot_survivors, data_addr, data_length);
//    break;

//    case EVENT_DATA_ID:
//      memcpy(&judge_recv_mesg.event_data, data_addr, data_length);
//    break;
//    
//    case SUPPLY_PROJECTILE_ACTION_ID:
//      memcpy(&judge_recv_mesg.supply_projectile_action, data_addr, data_length);
//    break;

//    case SUPPLY_PROJECTILE_BOOK_ID:
//      memcpy(&judge_recv_mesg.supply_projectile_booking, data_addr, data_length);
//    break;

//    case GAME_ROBOT_STATE_ID:
//      memcpy(&judge_recv_mesg.game_robot_state, data_addr, data_length);
//    break;
//    
//    case POWER_HEAT_DATA_ID:
//      memcpy(&judge_recv_mesg.power_heat_data, data_addr, data_length);
//    break;
//	
//		case ROBOT_POS_ID:
//			memcpy(&judge_recv_mesg.game_robot_pos, data_addr, data_length);
//		break;
//		
//		case BUFF_MUSK_ID:
//			memcpy(&judge_recv_mesg.buff_musk, data_addr, data_length);
//		break;
//			
//		case AERIAL_ROBOT_ENERGY_ID:
//			memcpy(&judge_recv_mesg.aerial_robot_energy, data_addr, data_length);
//		break;		
//			
//		case ROBOT_HURT_ID:
//			memcpy(&judge_recv_mesg.robot_hurt, data_addr, data_length);
//		break;		
//			
//		case SHOOT_DATA_ID:
//			memcpy(&judge_recv_mesg.shoot_data, data_addr, data_length);
//		break;			
//		
//		case STUDENT_INTERACTIVE_ID:
//		{
//			if(data_length <= 119)    //���ݶ�ͷ�ṹ����+�������ݳ���
//			{
//				memcpy(&judge_recv_mesg.student_interactive_header_data, data_addr, data_length);
//				interover_data_overflow = 0;
//			}
//			else
//			{
//				interover_data_overflow = 1;
//			}
//		}break;	

    case GAME_INFO_ID://0x0001              
      memcpy(&judge_recv_mesg.game_information, data_addr, data_length);
    break;

    case REAL_BLOOD_DATA_ID://0x0002  //û��
      memcpy(&judge_recv_mesg.blood_changed_data, data_addr, data_length);
    break;

		/**********design by li****************///û��
		case REAL_SHOOT_DATA_ID://0x0003
		{
//			shoot_one = 1;
			judge_recv_mesg.client_data.data1--;
      memcpy(&judge_recv_mesg.real_shoot_data, data_addr, data_length);
		}
    break;
		/**************************************/
		
    case REAL_POWER_DATA_ID://0x0004
      memcpy(&judge_recv_mesg.power_heat_data, data_addr, data_length);
    break;
    
    case REAL_FIELD_DATA_ID://0x0005 //û��
      memcpy(&judge_recv_mesg.rfid_data, data_addr, data_length);
    break;

    case GAME_RESULT_ID://0x0006 //û��
      memcpy(&judge_recv_mesg.game_result_data, data_addr, data_length);
    break;

    case GAIN_BUFF_ID://0x0007 //û��
      memcpy(&judge_recv_mesg.get_buff_data, data_addr, data_length);
    break;

		/*****design by li*********/
		case POSITION_DATA_ID://0x0008
      memcpy(&judge_recv_mesg.position, data_addr, data_length);
    break;
		/**************************/
		
		/*****design by li*********/ //�û��Զ�������//û��
//		case STU_CUSTOM_DATA_ID://0x0100
//      memcpy(&judge_rece_mesg.client_data, data_addr, data_length);
//    break;
		/**************************/
		
    case CLIENT_TO_ROBOT_ID://û��
      memcpy(&judge_recv_mesg.student_download_data, data_addr, data_length);
    break;
    
    default:
    {}
    break;
  }
  
}
