#include <boost/asio.hpp>
#include <iostream>
#include <pixhawk/mavlink.h>
#include "ros/ros.h"
#include <marvel_v_0_1/Autopilot.h>
#include <marvel_v_0_1/Guidance_Command.h>
#include <radio.h>
//
// Initializing boost
//
using namespace boost;
asio::io_service ios;
asio::serial_port port(ios);
//
// Message initialization
//
marvel_v_0_1::Autopilot autopilot_msg;
marvel_v_0_1::Guidance_Command guidance_msg;
//
// Global variables
//
bool last_arm_status = 0;
bool current_arm_status = 0;
int roll = 0, pitch = 0, yaw = 0, throttle = 0;
int mode = 0;
radio rc;
//
// Mavlink message receive and handle function
//
void msg_receive(uint8_t c) {
    mavlink_message_t msg;
    mavlink_status_t status;
    //
    // Try to get a new message
    //
    if(mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status))
    {   //
        // Handle message
        //
        std::cout << int(msg.msgid) <<std::endl;
		switch(msg.msgid)
        {   case MAVLINK_MSG_ID_HEARTBEAT:
				mavlink_heartbeat_t heartbeat;
				mavlink_msg_heartbeat_decode(&msg, &heartbeat);
				if(int(heartbeat.base_mode) == 81) autopilot_msg.armed = false;
				if(int(heartbeat.base_mode) == 209) autopilot_msg.armed = true;
			break;
            case MAVLINK_MSG_ID_ATTITUDE:
				mavlink_attitude_t attitude_msg;
				mavlink_msg_attitude_decode(&msg, &attitude_msg);
				autopilot_msg.rate = attitude_msg.yawspeed * 180 / 3.14;
			break;
			case MAVLINK_MSG_ID_VFR_HUD:
				mavlink_vfr_hud_t vfr_msg;
				mavlink_msg_vfr_hud_decode(&msg, &vfr_msg);
				autopilot_msg.heading = vfr_msg.heading;
				autopilot_msg.climb = vfr_msg.climb;
			break;
			case MAVLINK_MSG_ID_PARAM_VALUE:
				mavlink_param_value_t param_value_msg;
				mavlink_msg_param_value_decode(&msg, &param_value_msg);
				switch(int(param_value_msg.param_index)) {
					//
					// Extracting radio roll parameters
					//
					case 70:
					rc.roll.min = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 71:
					rc.roll.trim = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 72:
					rc.roll.max = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 73:
					rc.roll.rev = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 74:
					rc.roll.dz = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					//
					// Extracting radio pitch parameters
					//
					case 75:
					rc.pitch.min = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 76:
					rc.pitch.trim = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 77:
					rc.pitch.max = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 78:
					rc.pitch.rev = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 79:
					rc.pitch.dz = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					//
					// Extracting radio throttle parameters
					//
					case 80:
					rc.throttle.min = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 81:
					rc.throttle.trim = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 82:
					rc.throttle.max = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 83:
					rc.throttle.rev = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 84:
					rc.throttle.dz = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					//
					// Extracting radio yaw parameters
					//
					case 85:
					rc.yaw.min = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 86:
					rc.yaw.trim = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 87:
					rc.yaw.max = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 88:
					rc.yaw.rev = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 89:
					rc.yaw.dz = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					//
					// Extracting radio mode parameters
					//
					case 90:
					rc.mode.min = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 91:
					rc.mode.trim = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 92:
					rc.mode.max = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 93:
					rc.mode.rev = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 94:
					rc.mode.dz = int(mavlink_msg_param_value_get_param_value(&msg));
					break;
					
					case 500:
					std::cout << "Parameters received...!" << std::endl;
					break;
				}
			break;
        }
    }
}
//
// Mavlink radio message send function
//
void msg_send_radio(float throttle, float roll, float pitch, float yaw) {
	if(autopilot_msg.armed == true) {
		//
		// Command initializtion
		//
		mavlink_rc_channels_override_t radio_commands;
		radio_commands.target_system = 1;
		radio_commands.target_component = MAV_COMP_ID_SYSTEM_CONTROL;
		radio_commands.chan1_raw = roll;
		radio_commands.chan2_raw = pitch;
		radio_commands.chan3_raw = throttle;
		radio_commands.chan4_raw = yaw;
		radio_commands.chan5_raw = mode; 
		radio_commands.chan6_raw = 0; 
		radio_commands.chan7_raw = 0;
		radio_commands.chan8_raw = 0; 
		//
		// Message pack and send
		//
		mavlink_message_t msg;
		uint8_t buf[MAVLINK_MAX_PACKET_LEN];
		mavlink_msg_rc_channels_override_encode(255, 0, &msg, &radio_commands);
		unsigned len = mavlink_msg_to_send_buffer(buf, &msg);
		asio::write(port, asio::buffer(buf,len));	
	}
	if(autopilot_msg.armed == false) {
		//
		// Command initializtion
		//
		mavlink_rc_channels_override_t radio_commands;
		radio_commands.target_system = 1;
		radio_commands.target_component = MAV_COMP_ID_SYSTEM_CONTROL;
		radio_commands.chan1_raw = 0;
		radio_commands.chan2_raw = 0;
		radio_commands.chan3_raw = 0;
		radio_commands.chan4_raw = 0;
		radio_commands.chan5_raw = 0; 
		radio_commands.chan6_raw = 0; 
		radio_commands.chan7_raw = 0;
		radio_commands.chan8_raw = 0; 
		//
		// Message pack and send
		//
		mavlink_message_t msg;
		uint8_t buf[MAVLINK_MAX_PACKET_LEN];
		mavlink_msg_rc_channels_override_encode(255, 0, &msg, &radio_commands);
		unsigned len = mavlink_msg_to_send_buffer(buf, &msg);
		asio::write(port, asio::buffer(buf,len));	
	} 
}
// 
// Radio send timer callback function
//
void msg_send_radio_callback(const ros::TimerEvent&)
{
	msg_send_radio(throttle, roll, pitch, yaw);
}
//
// Mavlink arm message send function
//
void msg_send_arm() {
	//
	// Command initializtion
	//
	mavlink_command_long_t arm_command_msg;
	arm_command_msg.command = MAV_CMD_COMPONENT_ARM_DISARM;
    arm_command_msg.target_system = 1;
    arm_command_msg.target_component = MAV_COMP_ID_SYSTEM_CONTROL;
    arm_command_msg.confirmation = 0;
    arm_command_msg.param1 = 1;
    arm_command_msg.param2 = 0;
    arm_command_msg.param3 = 0;
    arm_command_msg.param4 = 0;
    arm_command_msg.param5 = 0;
    arm_command_msg.param6 = 0;
    arm_command_msg.param7 = 0;
	//
	// Message pack and send
	// 
	mavlink_message_t msg;
	uint8_t buf[MAVLINK_MAX_PACKET_LEN];
	mavlink_msg_command_long_encode(255, 0, &msg, &arm_command_msg);
	unsigned len = mavlink_msg_to_send_buffer(buf, &msg);
	asio::write(port, asio::buffer(buf,len));
}
//
// Mavlink disarm message send function
//
void msg_send_disarm() {
	//
	// Command initializtion
	//
	mavlink_command_long_t arm_command_msg;
	arm_command_msg.command = MAV_CMD_COMPONENT_ARM_DISARM;
    arm_command_msg.target_system = 1;
    arm_command_msg.target_component = MAV_COMP_ID_SYSTEM_CONTROL;
    arm_command_msg.confirmation = 0;
    arm_command_msg.param1 = 0;
    arm_command_msg.param2 = 0;
    arm_command_msg.param3 = 0;
    arm_command_msg.param4 = 0;
    arm_command_msg.param5 = 0;
    arm_command_msg.param6 = 0;
    arm_command_msg.param7 = 0;
	//
	// Message pack and send
	//
	mavlink_message_t msg;
	uint8_t buf[MAVLINK_MAX_PACKET_LEN];	
	mavlink_msg_command_long_encode(255, 0, &msg, &arm_command_msg);
	unsigned len = mavlink_msg_to_send_buffer(buf, &msg);
	asio::write(port, asio::buffer(buf,len));
}
//
// Mavlink heartbeat message send function
//
void msg_send_heartbeat() {
	//
	// Command initializtion
	//
	mavlink_heartbeat_t heartbeat_msg;
	heartbeat_msg.type = MAV_TYPE_GCS;
	heartbeat_msg.autopilot = MAV_AUTOPILOT_INVALID;
	heartbeat_msg.base_mode = 0;
	heartbeat_msg.custom_mode = 0;
	heartbeat_msg.system_status = 0;
	heartbeat_msg.mavlink_version = 3;
	//
	// Message pack and send
	// 
	mavlink_message_t msg;
	uint8_t buf[MAVLINK_MAX_PACKET_LEN];
	mavlink_msg_heartbeat_encode(255, 0, &msg, &heartbeat_msg);
	unsigned len = mavlink_msg_to_send_buffer(buf, &msg);
	asio::write(port, asio::buffer(buf,len));
	std::cout << "heartbeat sent" << std::endl;
}
//
// Heartbeat send timer callback function
//
void msg_send_heartbeat_callback(const ros::TimerEvent&)
{
	msg_send_heartbeat();
}
//
// Mavlink request parameter message send fucntion
//
void msg_send_request_param() {
	//
	// Command initializtion
	//
	mavlink_param_request_list_t request_param__list_msg;
	request_param__list_msg.target_system = 1;
	request_param__list_msg.target_component = MAV_COMP_ID_SYSTEM_CONTROL;	 
	//
	// Message pack and send
	//
	mavlink_message_t msg;
	uint8_t buf[MAVLINK_MAX_PACKET_LEN];	
	mavlink_msg_param_request_list_encode(255, 0, &msg, &request_param__list_msg);
	unsigned len = mavlink_msg_to_send_buffer(buf, &msg);
	asio::write(port, asio::buffer(buf,len));	
}
//
// Guidance message callback function
//
void guidance_msg_Callback(const marvel_v_0_1::Guidance_Command::ConstPtr& msg) {
	current_arm_status = msg->arm;
	throttle = rc.calc_throttle(int(msg->throttle));
	roll = rc.calc_roll(int(msg->roll));
	pitch = rc.calc_pitch(int(msg->pitch));
	yaw = rc.calc_yaw(int(msg->yaw));
	mode = rc.calc_mode(int(msg->mode));
}
//
// Main program
//
int main(int argc, char **argv) {


    printf( " ********************************************** \n" );
    printf( " ********************************************** \n" );
    printf( " *                                            * \n" );
    printf( " *                                            * \n" );
    printf( " *                                            * \n" );
    printf( " *                                            * \n" );
    printf( " *                                            * \n" );
    printf( " *                                            * \n" );
    printf( " *                                            * \n" );
    printf( " *     This program debug and Develop by      * \n" );
    printf( " *          Mohammad Hossein Kazemi           * \n" );
    printf( " *               Ali Jameie                   * \n" );
    printf( " *        All Right reserved 2015-2016        * \n" );
    printf( " *      Email:Mhkazemi_engineer@yahoo.com     * \n" );
    printf( " *        Email:Celarco.Group@Gmail.com       * \n" );
    printf( " *     AmirKabir University of Technology     * \n" );
    printf( " *   AUT-MAV AUTONOMOUS AIRIAL VEHICLE TEAM   * \n" );
    printf( " *                                            * \n" );
    printf( " *                                            * \n" );
    printf( " *                                            * \n" );
    printf( " *                                            * \n" );
    printf( " ********************************************** \n" );
    printf( " ********************************************** \n" );


    //
    // Port configuration
    //
    port.open("/dev/ttySAC0");
    port.set_option(asio::serial_port_base::baud_rate(115200));
    //
    // Ros configuration
    //
    ros::init(argc, argv, "server");
    ros::NodeHandle n;
    ros::Publisher pub = n.advertise<marvel_v_0_1::Autopilot>("server", 1000);
    ros::Subscriber sub = n.subscribe("guidance_pack", 1000, guidance_msg_Callback);
    ros::Timer rc_timer = n.createTimer(ros::Duration(0.1), msg_send_radio_callback);
	ros::Timer heartbeat_timer = n.createTimer(ros::Duration(1.0), msg_send_heartbeat_callback);
	//
	// Read autopilot parameters
	//
	//msg_send_request_param();
	//
    // Main loop
    //
	char r_byte;
    while(1) {
        asio::read(port, asio::buffer(&r_byte,1));
        msg_receive(uint8_t(r_byte));
		
	 	if(((current_arm_status == 1)&&(last_arm_status == 0)) || ((current_arm_status == 1)&&(autopilot_msg.armed == 0))) {
			last_arm_status = 1;
			msg_send_arm();
		}
		if((current_arm_status == 0)&&(last_arm_status == 1)) {
			last_arm_status = 0;
			msg_send_disarm();
		} 
		ros::spinOnce();
        pub.publish(autopilot_msg);
    }
    return 0;
}
