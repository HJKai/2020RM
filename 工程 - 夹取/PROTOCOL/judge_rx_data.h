#ifndef _judge_rx_data_H
#define _judge_rx_data_H


#include "stm32f4xx.h"
#include "data_packet.h"

#define JUDGE_RX_FIFO_BUFLEN 500


//typedef enum
//{
//	GAME_STATE_ID                = 0x0001,  //比赛状态数据
//	GAME_RESULT_ID 	             = 0x0002,  //比赛结果数据
//	ROBOT_SURVIVORS_ID           = 0x0003,	//比赛机器人存活数据
//	EVENT_DATA_ID 				       = 0x0101,	//场地事件数据
//	SUPPLY_PROJECTILE_ACTION_ID  = 0x0102,	//场地补给站动作标识数据
//	SUPPLY_PROJECTILE_BOOK_ID    = 0x0103,	//场地补给站预约子弹数据
//	GAME_ROBOT_STATE_ID          = 0x0201,	//机器人状态数据
//	POWER_HEAT_DATA_ID           = 0x0202,	//实时功率热量数据
//	ROBOT_POS_ID                 = 0x0203,	//机器人位置数据
//	BUFF_MUSK_ID                 = 0x0204,	//机器人增益数据
//	AERIAL_ROBOT_ENERGY_ID       = 0x0205,	//空中机器人能量状态数据
//	ROBOT_HURT_ID                = 0x0206,	//伤害状态数据
//	SHOOT_DATA_ID                = 0x0207,	//实时射击数据
//	STUDENT_INTERACTIVE_ID       = 0x0301,	//机器人间交互数据 和 客户端通信
//} judge_data_id_e;

///*裁判系统数据结构体*/

//typedef __packed struct
//{
// uint8_t game_type:4;
// uint8_t game_progress:4;
// uint16_t stage_remain_time;
//} ext_game_state_t;

//typedef __packed struct
//{
// uint8_t winner;
//} ext_game_result_t;

//typedef __packed struct
//{
// uint16_t robot_legion;
//} ext_game_robot_survivors_t;

//typedef __packed struct
//{
// uint32_t event_type;
//} ext_event_data_t;

//typedef __packed struct
//{
// uint8_t supply_projectile_id; 
// uint8_t supply_robot_id; 
// uint8_t supply_projectile_step; 
//	
// uint8_t supply_projectile_num; //1.1版本
//} ext_supply_projectile_action_t;

//typedef __packed struct
//{
// uint8_t supply_projectile_id;
// uint8_t supply_robot_id;    //1.1版本
// uint8_t supply_num;
//} ext_supply_projectile_booking_t;

//typedef __packed struct
//{
// uint8_t robot_id;
// uint8_t robot_level;
// uint16_t remain_HP;
// uint16_t max_HP;
// uint16_t shooter_heat0_cooling_rate;
// uint16_t shooter_heat0_cooling_limit;
// uint16_t shooter_heat1_cooling_rate;
// uint16_t shooter_heat1_cooling_limit;
// uint8_t mains_power_gimbal_output : 1;
// uint8_t mains_power_chassis_output : 1;
// uint8_t mains_power_shooter_output : 1;
//} ext_game_robot_state_t;

//typedef __packed struct
//{
// uint16_t chassis_volt; 
// uint16_t chassis_current; 
// float chassis_power; 
// uint16_t chassis_power_buffer; 
// uint16_t shooter_heat0; 
// uint16_t shooter_heat1; 
//} ext_power_heat_data_t;


//typedef __packed struct
//{
// float x;
// float y;
// float z;
// float yaw;
//} ext_game_robot_pos_t;

//typedef __packed struct
//{
// uint8_t power_rune_buff;
//}ext_buff_musk_t;

//typedef __packed struct
//{
// uint8_t energy_point;
// uint8_t attack_time;
//} aerial_robot_energy_t;

//typedef __packed struct
//{
// uint8_t armor_id : 4;
// uint8_t hurt_type : 4;
//} ext_robot_hurt_t;

//typedef __packed struct
//{
// uint8_t bullet_type;
// uint8_t bullet_freq;
// float bullet_speed;
//} ext_shoot_data_t;

//typedef __packed struct
//{
// uint16_t data_cmd_id;
// uint16_t send_ID;
// uint16_t receiver_ID;
//	
// uint8_t interactive_data[50];
//}ext_RX_student_interactive_header_data_t;

//typedef struct
//{
//  ext_game_state_t                      	    game_state;               	 	   //0x0001
//  ext_game_result_t                      	    game_result;               	     //0x0002
//  ext_game_robot_survivors_t       			 	    game_robot_survivors;     		   //0x0003
//  ext_event_data_t  										      event_data;                	     //0x0101
//  ext_supply_projectile_action_t     	        supply_projectile_action;  	     //0x0102
//  ext_supply_projectile_booking_t     	      supply_projectile_booking; 	     //0x0103
//  ext_game_robot_state_t        		          game_robot_state;      		       //0x0201
//  ext_power_heat_data_t   				            power_heat_data;     		 	       //0x0202
//  ext_game_robot_pos_t   				              game_robot_pos;				           //0x0203
//  ext_buff_musk_t						                  buff_musk;						           //0x0204
//  aerial_robot_energy_t					              aerial_robot_energy;			       //0x0205
//  ext_robot_hurt_t						                robot_hurt;						           //0x0206
//  ext_shoot_data_t						                shoot_data;			           	     //0x0207
//  ext_RX_student_interactive_header_data_t	  student_interactive_header_data; //0x0301

//} judge_rxdata_t;


//extern judge_rxdata_t judge_recv_mesg;
extern uart_dma_rxdata_t judge_rx_obj;
extern unpack_data_t judge_unpack_obj;

typedef enum
{
  GAME_INFO_ID       = 0x0001,  //10Hz
  REAL_BLOOD_DATA_ID = 0x0002,
  REAL_SHOOT_DATA_ID = 0x0003,
	REAL_POWER_DATA_ID = 0x0004,   //实时功率和热量数据
  REAL_FIELD_DATA_ID = 0x0005,  //10hZ
  GAME_RESULT_ID     = 0x0006,
  GAIN_BUFF_ID       = 0x0007,
	POSITION_DATA_ID   = 0x0008,  //机器人位置和枪口朝向信息，50HZ频率周期发送
  
  STU_CUSTOM_DATA_ID = 0x0100,
  ROBOT_TO_CLIENT_ID = 0x0101,
  CLIENT_TO_ROBOT_ID = 0x0102,
} judge_data_id_e;


/** 
  * @brief  GPS state structures definition
  */
typedef __packed struct
{
  uint8_t valid_flag;
  float x;
  float y;
  float z;
  float yaw;
} position_t;

/** 
  * @brief  game information structures definition(0x0001)
  *         this package send frequency is 50Hz
  */
typedef __packed struct
{
  uint16_t   stage_remain_time;
  uint8_t    game_process;
  /* current race stage
   0 not start
   1 preparation stage
   2 self-check stage
   3 5 seconds count down
   4 fighting stage
   5 result computing stage */
 // uint8_t    reserved;
	uint8_t    robotLevel;
  uint16_t   remain_hp;
  uint16_t   max_hp;
 // position_t position;
} game_robot_state_t;

/** 
  * @brief  real time blood volume change data(0x0002)
  */
typedef __packed struct
{
  uint8_t armor_type:4;
 /* 0-3bits: the attacked armor id:
    0x00: 0 front
    0x01：1 left
    0x02：2 behind
    0x03：3 right
    others reserved*/
  uint8_t hurt_type:4;
 /* 4-7bits: blood volume change type
    0x00: armor attacked
    0x01：module offline
    0x02: bullet over speed
    0x03: bullet over frequency */
} robot_hurt_data_t;

/** 
  * @brief  real time shooting data(0x0003)
  */
typedef __packed struct
{
 // uint8_t reserved1;
	uint8_t bulletType;  //弹丸类型
  uint8_t bullet_freq;
  float   bullet_spd;
 // float   reserved2;
	
} real_shoot_t;

/*********design by li********/
/** 
  * @brief  power_heat_data(0x0004)
  */
typedef __packed struct
{
 float chassisVolt;    //底盘输出电压
 float chassisCurrent; //底盘输出电流
 float chassisPower;   //底盘输出功率
 float chassisPowerBuffer; //地盘功率缓冲
 uint16_t shootHeat0;      //17mm枪口热量
 uint16_t shootHeat1;      //42mm枪口热量
}extPowerHeatData_t;
/*****************************/

/** 
  * @brief  rfid detect data(0x0005)
  */
typedef __packed struct
{
  uint8_t card_type;
  uint8_t card_idx;
} rfid_detect_t;

/** 
  * @brief  game result data(0x0006)
  */
typedef __packed struct
{
  uint8_t winner;
} game_result_t;

/** 
  * @brief  the data of get field buff(0x0007)
  */
typedef __packed struct
{
	/*
  uint8_t buff_type;
  uint8_t buff_addition;
	*/
	uint16_t buffMusk;
} get_buff_t;

/**********design by li**************/
/** 
  * @brief  position(0x0008)
  */
typedef __packed struct
{
  //uint8_t valid_flag;
  float x;  //位置x坐标值
  float y;	//位置y坐标值
  float z;	//位置z坐标值
  float yaw; //枪口朝向角度值
} extGameRobotPos_t;
/*********************************/

/** 
  * @brief  student custom data
  */
typedef __packed struct
{
  float data1;
  float data2;
  float data3;
	uint8_t mask;  //自定义数据4
} client_show_data_t;

typedef __packed struct
{
  uint8_t  data[64];
} user_to_server_t;

typedef __packed struct
{
  uint8_t  data[32];
} server_to_user_t;

/** 
  * @brief  the data structure receive from judgement
  */
typedef struct
{
  game_robot_state_t game_information;
  robot_hurt_data_t  blood_changed_data;
  real_shoot_t       real_shoot_data;
	extPowerHeatData_t power_heat_data; //
  rfid_detect_t      rfid_data;
  game_result_t      game_result_data;
  get_buff_t         get_buff_data;
	extGameRobotPos_t  position;        //
	client_show_data_t client_data;     //用户自定义数据
  server_to_user_t   student_download_data;
} receive_judge_t;

extern receive_judge_t judge_recv_mesg;

////////////////////////
void judgement_rx_param_init(void);
void judgement_data_handler(uint8_t *p_frame);

#endif

