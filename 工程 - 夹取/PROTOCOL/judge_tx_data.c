#include "judge_tx_data.h"
#include "dma.h"
#include "judge_rx_data.h"
#include "string.h"

judge_txdata_t judge_send_mesg;
/*裁判系统发送*/
static SemaphoreHandle_t judge_txdata_mutex;
fifo_s_t  judge_txdata_fifo;
static uint8_t   judge_txdata_buf[JUDGE_TX_FIFO_BUFLEN];

void judgement_tx_param_init(void)
{
  /* create the judge_rxdata_mutex mutex  */  
  judge_txdata_mutex = xSemaphoreCreateMutex();
  
  /* judge data fifo init */
  fifo_s_init(&judge_txdata_fifo, judge_txdata_buf, JUDGE_TX_FIFO_BUFLEN, judge_txdata_mutex);
}

void judgement_client_packet_pack(client_custom_data_t *p_data)
{	
//	client_custom_data_t *frame = (client_custom_data_t *) p_data;
//	memcpy(frame, p_data, sizeof(client_custom_data_t));
//	
//	uint8_t current_robot_id = 0;
//	uint16_t receiver_ID = 0;
//	
//	current_robot_id = judge_recv_mesg.game_robot_state.robot_id;  //读取当前机器人的id
//	switch(current_robot_id)
//	{
//		//red robot
//		case STUDENT_RED_HERO_ID:
//		{
//			receiver_ID = RED_HERO_CLIENT_ID;
//		}break;
//		case STUDENT_RED_ENGINEER_ID:
//		{
//			receiver_ID = RED_ENGINEER_CLIENT_ID;
//		}break;
//		case STUDENT_RED_AERIAL_ID:
//		{
//			receiver_ID = RED_AERIAL_CLIENT_ID;
//		}break;

//		case STUDENT_RED_INFANTRY3_ID:
//		{
//			receiver_ID = RED_INFANTRY3_CLIENT_ID;
//		}break;
//		case STUDENT_RED_INFANTRY4_ID:
//		{
//			receiver_ID = RED_INFANTRY4_CLIENT_ID;
//		}break;
//		case STUDENT_RED_INFANTRY5_ID:
//		{
//			receiver_ID = RED_INFANTRY5_CLIENT_ID;
//		}break;
//		
//		//blue robot
//		case STUDENT_BLUE_HERO_ID:
//		{
//			receiver_ID = BLUE_HERO_CLIENT_ID;
//		}break;
//		case STUDENT_BLUE_ENGINEER_ID:
//		{
//			receiver_ID = BLUE_ENGINEER_CLIENT_ID;
//		}break;
//		case STUDENT_BLUE_AERIAL_ID:
//		{
//			receiver_ID = BLUE_AERIAL_CLIENT_ID;
//		}break;
//		
//		case STUDENT_BLUE_INFANTRY3_ID:
//		{
//			receiver_ID = BLUE_INFANTRY3_CLIENT_ID;
//		}break;	
//		case STUDENT_BLUE_INFANTRY4_ID:
//		{
//			receiver_ID = BLUE_INFANTRY4_CLIENT_ID;
//		}break;
//		case STUDENT_BLUE_INFANTRY5_ID:
//		{
//			receiver_ID = BLUE_INFANTRY5_CLIENT_ID;
//		}break;

//	}

//	judge_send_mesg.student_interactive_header_data.data_cmd_id = CLIENT_DATA_ID;		//0xd180 客户端自定义数据
//	judge_send_mesg.student_interactive_header_data.send_ID = current_robot_id;
//	judge_send_mesg.student_interactive_header_data.receiver_ID = receiver_ID;
//	
//	/*添加自定义数据*/
////	judge_send_mesg.client_custom_data.data1 = shoot.fric_wheel_spd + shoot.para_mode * 100000;
////	judge_send_mesg.client_custom_data.data2 = chassis.CapData[1];
////	judge_send_mesg.client_custom_data.masks = masks_updata();	
//	
//	memcpy(&judge_send_mesg.student_interactive_header_data.interactive_data[0],(uint8_t*) p_data,sizeof(client_custom_data_t));
//	
//	data_packet_pack(STUDENT_INTERACTIVE_ID, (uint8_t *)&judge_send_mesg.student_interactive_header_data,
//									 CLIENT_DATA_LENGTH + 6, DN_REG_ID);
}

uint16_t debug_radius = 12;
uint16_t debug_start_xx = 750;
uint16_t debug_start_yy = 40;
uint16_t debug_width = 2;
uint16_t debug_start_x = 960;
uint16_t debug_start_y = 735;
		
char text1[30] = {"DCG         MCL "};

static void client_graphic_draw_TXT (void)
{
	memcpy(judge_send_mesg.client_graphic_draw.text,text1,sizeof(text1));
	judge_send_mesg.client_graphic_draw.color =  PINK ;
	judge_send_mesg.client_graphic_draw.graphic_name[0] = TXT;
	judge_send_mesg.client_graphic_draw.graphic_tpye = TEXT;
	judge_send_mesg.client_graphic_draw.layer = 2;
	judge_send_mesg.client_graphic_draw.operate_tpye = ADD_GRAPHICS;
	judge_send_mesg.client_graphic_draw.radius = debug_radius;
	judge_send_mesg.client_graphic_draw.start_x = debug_start_x+debug_start_xx;
	judge_send_mesg.client_graphic_draw.start_y = debug_start_y+debug_start_yy;
	judge_send_mesg.client_graphic_draw.width = debug_width;
	judge_send_mesg.client_graphic_draw.text_lenght = sizeof(text1);

}

static void client_graphic_draw_roundness_middle1 (void)  //竖向
{
	judge_send_mesg.client_graphic_draw.color 						=  PINK ;
	judge_send_mesg.client_graphic_draw.graphic_name[0] 	= LINK1;
	judge_send_mesg.client_graphic_draw.graphic_tpye 			= STAIGHT_LINE;
	judge_send_mesg.client_graphic_draw.layer 						= 1;
	judge_send_mesg.client_graphic_draw.operate_tpye			= ADD_GRAPHICS;
	judge_send_mesg.client_graphic_draw.start_x 					= debug_start_x;
	judge_send_mesg.client_graphic_draw.start_y 					= debug_start_y;
	judge_send_mesg.client_graphic_draw.end_x 						= debug_start_x;
	judge_send_mesg.client_graphic_draw.end_y	 						= debug_start_y-240;
	judge_send_mesg.client_graphic_draw.width 						= debug_width;

}

static void client_graphic_draw_roundness_middle2 (void)//第一条
{
	judge_send_mesg.client_graphic_draw.color 					=  PINK ;
	judge_send_mesg.client_graphic_draw.graphic_name[0] = LINK2;
	judge_send_mesg.client_graphic_draw.graphic_tpye 		= STAIGHT_LINE;
	judge_send_mesg.client_graphic_draw.layer						= 1;
	judge_send_mesg.client_graphic_draw.operate_tpye 		= ADD_GRAPHICS;
	judge_send_mesg.client_graphic_draw.start_x					= debug_start_x;
	judge_send_mesg.client_graphic_draw.start_y 				= debug_start_y-40;
	judge_send_mesg.client_graphic_draw.end_x 					= debug_start_x-50;
	judge_send_mesg.client_graphic_draw.end_y 					= debug_start_y-40;
	judge_send_mesg.client_graphic_draw.width 					= debug_width;
	
}
static void client_graphic_draw_roundness_middle3 (void)	//第二条
{
	judge_send_mesg.client_graphic_draw.color 						=  PINK ;
	judge_send_mesg.client_graphic_draw.graphic_name[0] 	= LINK3;
	judge_send_mesg.client_graphic_draw.graphic_tpye 			= STAIGHT_LINE;
	judge_send_mesg.client_graphic_draw.layer 						= 1;
	judge_send_mesg.client_graphic_draw.operate_tpye			= ADD_GRAPHICS;
	judge_send_mesg.client_graphic_draw.start_x 					= debug_start_x;
	judge_send_mesg.client_graphic_draw.start_y 					= debug_start_y-80;
	judge_send_mesg.client_graphic_draw.end_x 						= debug_start_x-100;
	judge_send_mesg.client_graphic_draw.end_y	 						= debug_start_y-80;
	judge_send_mesg.client_graphic_draw.width 						= debug_width;

}

static void client_graphic_draw_roundness_middle4 (void)	//第三条
{
	judge_send_mesg.client_graphic_draw.color 						=  PINK ;
	judge_send_mesg.client_graphic_draw.graphic_name[0] 	= LINK4;
	judge_send_mesg.client_graphic_draw.graphic_tpye 			= STAIGHT_LINE;
	judge_send_mesg.client_graphic_draw.layer 						= 1;
	judge_send_mesg.client_graphic_draw.operate_tpye			= ADD_GRAPHICS;
	judge_send_mesg.client_graphic_draw.start_x 					= debug_start_x;
	judge_send_mesg.client_graphic_draw.start_y 					= debug_start_y-120;
	judge_send_mesg.client_graphic_draw.end_x 						= debug_start_x-50;
	judge_send_mesg.client_graphic_draw.end_y	 						= debug_start_y-120;
	judge_send_mesg.client_graphic_draw.width 						= debug_width;

}

static void client_graphic_draw_roundness_middle5 (void) //第四条
{
	judge_send_mesg.client_graphic_draw.color 						=  PINK ;
	judge_send_mesg.client_graphic_draw.graphic_name[0] 	= LINK5;
	judge_send_mesg.client_graphic_draw.graphic_tpye 			= STAIGHT_LINE;
	judge_send_mesg.client_graphic_draw.layer 						= 1;
	judge_send_mesg.client_graphic_draw.operate_tpye			= ADD_GRAPHICS;
	judge_send_mesg.client_graphic_draw.start_x 					= debug_start_x;
	judge_send_mesg.client_graphic_draw.start_y 					= debug_start_y-160;
	judge_send_mesg.client_graphic_draw.end_x 						= debug_start_x-100;
	judge_send_mesg.client_graphic_draw.end_y	 						= debug_start_y-160;
	judge_send_mesg.client_graphic_draw.width 						= debug_width;

}

static void client_graphic_draw_roundness_middle6 (void)	//第五条
{
	judge_send_mesg.client_graphic_draw.color 						=  PINK ;
	judge_send_mesg.client_graphic_draw.graphic_name[0] 	= LINK6;
	judge_send_mesg.client_graphic_draw.graphic_tpye 			= STAIGHT_LINE;
	judge_send_mesg.client_graphic_draw.layer 						= 1;
	judge_send_mesg.client_graphic_draw.operate_tpye			= ADD_GRAPHICS;
	judge_send_mesg.client_graphic_draw.start_x 					= debug_start_x;
	judge_send_mesg.client_graphic_draw.start_y 					= debug_start_y-200;
	judge_send_mesg.client_graphic_draw.end_x 						= debug_start_x-50;
	judge_send_mesg.client_graphic_draw.end_y	 						= debug_start_y-200;
	judge_send_mesg.client_graphic_draw.width 						= debug_width;

}
static void client_graphic_draw_roundness_middle7 (void) //第六条
{
	judge_send_mesg.client_graphic_draw.color 						=  PINK ;
	judge_send_mesg.client_graphic_draw.graphic_name[0] 	= LINK7;
	judge_send_mesg.client_graphic_draw.graphic_tpye 			= STAIGHT_LINE;
	judge_send_mesg.client_graphic_draw.layer 						= 1;
	judge_send_mesg.client_graphic_draw.operate_tpye			= ADD_GRAPHICS;
	judge_send_mesg.client_graphic_draw.start_x 					= debug_start_x;
	judge_send_mesg.client_graphic_draw.start_y 					= debug_start_y;
	judge_send_mesg.client_graphic_draw.end_x 						= debug_start_x-100;
	judge_send_mesg.client_graphic_draw.end_y	 						= debug_start_y;
	judge_send_mesg.client_graphic_draw.width 						= debug_width;

}


uint16_t sta10 = 1;
void judgement_client_graphics_draw_pack(void)
{	
//	
//	uint8_t current_robot_id = 0;
//	uint16_t receiver_ID = 0;

////	uint8_t *puck_buff;
//	
//	current_robot_id = judge_recv_mesg.game_robot_state.robot_id;  //读取当前机器人的id
//	switch(current_robot_id)
//	{
//		//red robot
//		case STUDENT_RED_HERO_ID:
//		{
//			receiver_ID = RED_HERO_CLIENT_ID;
//		}break;
//		case STUDENT_RED_ENGINEER_ID:
//		{
//			receiver_ID = RED_ENGINEER_CLIENT_ID;
//		}break;
//		case STUDENT_RED_AERIAL_ID:
//		{
//			receiver_ID = RED_AERIAL_CLIENT_ID;
//		}break;

//		case STUDENT_RED_INFANTRY3_ID:
//		{
//			receiver_ID = RED_INFANTRY3_CLIENT_ID;
//		}break;
//		case STUDENT_RED_INFANTRY4_ID:
//		{
//			receiver_ID = RED_INFANTRY4_CLIENT_ID;
//		}break;
//		case STUDENT_RED_INFANTRY5_ID:
//		{
//			receiver_ID = RED_INFANTRY5_CLIENT_ID;
//		}break;
//		
//		//blue robot
//		case STUDENT_BLUE_HERO_ID:
//		{
//			receiver_ID = BLUE_HERO_CLIENT_ID;
//		}break;
//		case STUDENT_BLUE_ENGINEER_ID:
//		{
//			receiver_ID = BLUE_ENGINEER_CLIENT_ID;
//		}break;
//		case STUDENT_BLUE_AERIAL_ID:
//		{
//			receiver_ID = BLUE_AERIAL_CLIENT_ID;
//		}break;
//		case STUDENT_BLUE_INFANTRY3_ID:
//		{
//			receiver_ID = BLUE_INFANTRY3_CLIENT_ID;
//		}break;	
//		case STUDENT_BLUE_INFANTRY4_ID:
//		{
//			receiver_ID = BLUE_INFANTRY4_CLIENT_ID;
//		}break;
//		case STUDENT_BLUE_INFANTRY5_ID:
//		{
//			receiver_ID = BLUE_INFANTRY5_CLIENT_ID;
//		}break;

//	}

//	judge_send_mesg.student_interactive_header_data.data_cmd_id = CLIENT_PATTERN_ID;		//0xd180 客户端自定义数据
//	judge_send_mesg.student_interactive_header_data.send_ID = current_robot_id;
//	judge_send_mesg.student_interactive_header_data.receiver_ID = receiver_ID;

//	if(sta10 ==1)
//	{	
//		client_graphic_draw_roundness_middle1();
//	}
//	if(sta10 == 2)
//	{		
//		client_graphic_draw_roundness_middle2();
//	}
//	if(sta10 == 3)
//	{
//		client_graphic_draw_roundness_middle3();
//	}
//		if(sta10 ==4)
//	{	
//		client_graphic_draw_roundness_middle4();
//	}
//	if(sta10 == 5)
//	{		
//		client_graphic_draw_roundness_middle5();
//	}
//	if(sta10 == 6)
//	{
//		client_graphic_draw_roundness_middle6();
//	}
//	if(sta10 == 7)
//	{
//		client_graphic_draw_roundness_middle7();
//	}
//	if(sta10 == 8)
//	{
//		client_graphic_draw_TXT ();
//	}
//	memcpy(&judge_send_mesg.student_interactive_header_data.interactive_data[0],(uint8_t*)&judge_send_mesg.client_graphic_draw ,sizeof(judge_send_mesg.client_graphic_draw));
//	data_packet_pack(STUDENT_INTERACTIVE_ID, (uint8_t *)&judge_send_mesg.student_interactive_header_data,
//									 sizeof(judge_send_mesg.client_graphic_draw) + 6, DN_REG_ID);
}

