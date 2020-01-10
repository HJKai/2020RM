#ifndef _judge_tx_data_H
#define _judge_tx_data_H


#include "stm32f4xx.h"
#include "data_packet.h"

#define JUDGE_TX_FIFO_BUFLEN 500

#define STUDENT_DATA_LENGTH   sizeof(robot_interactive_data_t)
#define CLIENT_DATA_LENGTH   	sizeof(client_custom_data_t) 

//ѧ������id
typedef enum
{
	//ѧ������������id
	STUDENT_RED_HERO_ID       = 1,
	STUDENT_RED_ENGINEER_ID	  = 2,
	STUDENT_RED_INFANTRY3_ID  = 3,
	STUDENT_RED_INFANTRY4_ID  = 4,
	STUDENT_RED_INFANTRY5_ID  = 5,
	STUDENT_RED_AERIAL_ID	    = 6,
	STUDENT_RED_SENTRY_ID	    = 7,
	STUDENT_BLUE_HERO_ID	    = 11,
	STUDENT_BLUE_ENGINEER_ID  = 12,
	STUDENT_BLUE_INFANTRY3_ID = 13,
	STUDENT_BLUE_INFANTRY4_ID = 14,
	STUDENT_BLUE_INFANTRY5_ID = 15,
	STUDENT_BLUE_AERIAL_ID    = 16,
	STUDENT_BLUE_SENTRY_ID	  = 17,
	
	//�����˶�Ӧ�ͻ���id
	RED_HERO_CLIENT_ID	      = 0x0101,
	RED_ENGINEER_CLIENT_ID    = 0x0102,
	RED_INFANTRY3_CLIENT_ID	  = 0x0103,
	RED_INFANTRY4_CLIENT_ID	  = 0x0104,
	RED_INFANTRY5_CLIENT_ID   = 0x0105,
	RED_AERIAL_CLIENT_ID      = 0x0106,
	BLUE_HERO_CLIENT_ID	      = 0x0111,
	BLUE_ENGINEER_CLIENT_ID   = 0x0112,
	BLUE_INFANTRY3_CLIENT_ID  = 0x0113,
	BLUE_INFANTRY4_CLIENT_ID  = 0x0114,
	BLUE_INFANTRY5_CLIENT_ID  = 0x0115,
	BLUE_AERIAL_CLIENT_ID	    = 0x0116,
	
	//�Զ�������id
	CLIENT_DATA_ID	          = 0xd180,
	CLIENT_PATTERN_ID					= 0x0100,		//�ͻ����Զ���ͼ��
	
}interactive_id_e;

typedef enum
{
	ROUND										= 0x01,			//�Զ�����������
	LINK1										= 0x02,			//3�����
	LINK2										= 0x03,			//5�����
	LINK3										= 0x04,			//7�����
	LINK4										= 0x05,
	LINK5										= 0x06,
	LINK6										= 0x07,
	LINK7										= 0x08,
	LINK8										= 0x09,
	TXT											= 0X10,
}client_graphic_draw_name_e;

typedef enum
{
	NULL_GRAPHICS							= 0x00,			//��
	ADD_GRAPHICS							= 0x01,			//����ͼ��
	REMOVE_GRAPHICS						= 0x02,			//�޸�ͼ��
	MODIFY_GRAPHICS						= 0x04,			//ɾ��һ��ͼ��
	DELETE_LAYER_GRAPHICS     = 0x05,			//ɾ��һ��ͼ��
	DELETA_ALL_GRAOGICS				= 0x06,			//ɾ������ͼ��
}client_graphic_draw_operate_tpye_e;

typedef enum
{
	NULL_FORM									= 0x00,			//��
	STAIGHT_LINE							= 0x01,			//ֱ��
	RECTANGLE									= 0x02,			//����
	ROUNDNESS									= 0x03,			//Բ
	ELLIPSE										= 0x04,			//��Բ
	ARC										    = 0x05,			//����
	TEXT											= 0x06,			//�ı���ASCII����
}client_graphic_draw_graphic_tpye_e;


typedef enum
{
	RED_BULE									= 0x00,	//�Է���ɫ
	YELLOW										= 0x01,	//��ɫ
	GREEN											= 0x02,	//��ɫ
	ORANGE										= 0x03,	//��ɫ
	AMARANTH							    = 0x04, //�Ϻ�ɫ
	PINK											= 0x05,	//��ɫ
	CYAN											=	0x06,	//��ɫ
	BLANK											=	0x07,	//��ɫ
	WHITE											= 0x08, //��ɫ
}client_graphic_draw_color_e;
/* ��̨�͵�����ԽǶ�
 *
 *
*/
typedef enum
{	//mask ������ �ӵ͵��� �� uint8 �洢�෴
	NAGETIVE_30		=	0x04,		//0000 0100
	NAGETIVE_60		=	0x06,		//0000 0110
	NAGETIVE_90		=	0x07,		//0000 0111
	NAGETIVE_120	=	0x27,		//0010 0111
	NAGETIVE_150	=	0X37,		//0011 0111
	NAGETIVE_180	= 0x3f,		//0011 1111
	
	POSITIVE_180	= 0x3f,		//0011 1111
	POSITIVE_150	= 0X3b,		//0011 1011
	POSITIVE_120	= 0x39,		//0011 1001
	POSITIVE_90		= 0x38,		//0011 1000
	POSITIVE_60		= 0x18,		//0001 1000
	POSITIVE_30		= 0x08		//0000 1000
	
} yaw_relative_angle_2masks;

typedef __packed struct
{
 uint16_t data_cmd_id;
 uint16_t send_ID;
 uint16_t receiver_ID;
	
 uint8_t interactive_data[50];
}ext_TX_student_interactive_header_data_t;

typedef __packed struct
{
 float data1;
 float data2;
 float data3;
uint8_t masks;			//������ ��λ����λ
} client_custom_data_t;

typedef __packed struct
{
 uint8_t data[50];//113
} robot_interactive_data_t;

typedef __packed struct
	{ 
		uint8_t operate_tpye;  		//ͼ�β���
		uint8_t graphic_tpye; 		//ͼ������
		uint8_t graphic_name[5];  //ͼ����
		uint8_t layer; 						//ͼ��
		uint8_t color;  					//��ɫ	
		uint8_t width;						//�߿�
		uint16_t start_x; 				//��ʼX������
		uint16_t start_y; 				//��ʼY������
		uint16_t radius;  				//�����С���߰뾶
		uint16_t end_x;  					//�յ�X������
		uint16_t end_y; 					//�յ�Y������
		int16_t start_angle; 			//��ʼ�Ƕ�	Բ������ʼ�Ƕȣ� Բ��˳ʱ����ƣ���λΪ�ȣ���Χ[-180,180
		int16_t end_angle; 				//��ֹ�Ƕ�	Բ������ֹ�Ƕȣ���λΪ�ȣ���Χ[-180,180]
		uint8_t text_lenght; 			//�ı�����
		uint8_t text[30]; 				//�ı��ַ�
	} ext_client_graphic_draw_t ;
/** 
  * @brief  the data structure receive from judgement
  */
typedef struct
{
  ext_TX_student_interactive_header_data_t	  student_interactive_header_data; //0x0301
  client_custom_data_t					              client_custom_data;				       //0xD180
  robot_interactive_data_t				            robot_interactive_data;			     //0x0200~ox02FF
	ext_client_graphic_draw_t								    client_graphic_draw;							//0x0100
} judge_txdata_t;

typedef struct
{
	uint8_t		robot_id;
	uint8_t		robot_level;
	float			bullet_num;			//����ʣ���ӵ�
}user_judge_t;

extern judge_txdata_t judge_send_mesg;
extern fifo_s_t  judge_txdata_fifo;

void judgement_tx_param_init(void);
void judgement_client_packet_pack(client_custom_data_t *p_data);
void judgement_client_graphics_draw_pack(void);

#endif 

