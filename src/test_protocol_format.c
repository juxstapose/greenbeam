#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pack.h"
#include "session.h"
#include "session_hashtable.h"
#include "protocol.h"
#include "protocol_format.h"

void Test_Ping_Login_Response_Format_No_Sessions(LogConfig* log_config) {

	unsigned int initial_size = 20;
	
	//in range and out of range sessions that are logged in
	Session_Hashtable* session_hashtable_inrange = Session_Hashtable_Create(initial_size);
	Session_Hashtable* session_hashtable_outofrange = Session_Hashtable_Create(initial_size);
	
	unsigned int inrange_payload_body_size = 0;
	unsigned int inrange_num_items = 0;		
	Session_Hashtable_Calc_Size_Items(session_hashtable_inrange, &inrange_payload_body_size, &inrange_num_items);
	Log_log(log_config, LOG_DEBUG, "in range payload body size: %i inrange num_items:%i\n", inrange_payload_body_size, inrange_num_items);
	unsigned char* inrange_data = Session_Hashtable_To_Binary(session_hashtable_inrange, inrange_payload_body_size, inrange_num_items); 
	
	Log_log(log_config, LOG_DEBUG, "get binary form of out of range hashtable\n");
	unsigned int outofrange_payload_body_size = 0;
	unsigned int outofrange_num_items = 0;		
	Session_Hashtable_Calc_Size_Items(session_hashtable_outofrange, &outofrange_payload_body_size, &outofrange_num_items);
	Log_log(log_config, LOG_DEBUG, "out of range payload body size: %i out of range num_items:%i\n", outofrange_payload_body_size, outofrange_num_items);
	unsigned char* outofrange_data = Session_Hashtable_To_Binary(session_hashtable_outofrange, outofrange_payload_body_size, outofrange_num_items); 

	//calc data sizes	
	unsigned int outofrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
	unsigned int outofrange_data_size = outofrange_header_size + outofrange_payload_body_size;
	unsigned int inrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
	unsigned int inrange_data_size = inrange_header_size + outofrange_payload_body_size;
		
	Log_log(log_config, LOG_DEBUG, "inrange size: %i outofrange size: %i\n", inrange_data_size, outofrange_data_size);
	
	char* session_token = "ddsafndasnfsdjnferf934";
		
	//send in logged in response back to client
	unsigned char* data = Protocol_Login_Response(session_token, inrange_data_size, inrange_data, outofrange_data_size, outofrange_data);
	char* format = Protocol_Format_Get(data);
	Log_log(log_config, LOG_DEBUG, "format: %s\n", format);	
	unsigned int size_to_send = Binary_Calcsize(format);
	
	int test_size = Binary_Calcsize(HEADER_FORMAT) + inrange_data_size + outofrange_data_size;
	Log_log(log_config, LOG_DEBUG, "size to send:%i == size: %i\n", size_to_send, test_size);
	
	if(size_to_send == test_size) {
		Log_log(log_config, LOG_INFO, "test ping login no sessions response passed\n");
	}
}

void Test_Ping_Login_Response_Format_Multiple_Sessions_One(LogConfig* log_config) {
	unsigned int initial_size = 20;
	
	//in range and out of range sessions that are logged in
	Session_Hashtable* session_hashtable_inrange = Session_Hashtable_Create(initial_size);
	Session_Hashtable* session_hashtable_outofrange = Session_Hashtable_Create(initial_size);
	
	char* session_token_one = "afjodjossdjoasi";
	char* username_one = "juxstapose";
	Location* loc_one = Location_Create(0, 0, "starting_zone", 15, 10);
	Session* session_one = Session_Create(session_token_one, username_one, loc_one, NULL, NULL, NULL); 

	char* session_token_two = "afjodkldfdjdsddjossdjoasi";
	char* username_two = "bob";
	Location* loc_two = Location_Create(0, 0, "starting_zone", 7, 3);
	Session* session_two = Session_Create(session_token_two, username_two, loc_two, NULL, NULL, NULL); 
	
	Session_Hashtable_Set(session_hashtable_inrange, username_one, session_one);
	Session_Hashtable_Set(session_hashtable_inrange, username_two, session_two);
	
	char* inrange_str = Session_Hashtable_String(session_hashtable_inrange);
	
	Log_log(log_config, LOG_DEBUG, "inrange: %s\n", inrange_str);

	char* session_token_three = "afjodksdfsdkfksdfldfdjdsddjossdjoasi";
	char* username_three = "joe";
	Location* loc_three = Location_Create(0, 0, "starting_zone", 27, 23);
	Session* session_three = Session_Create(session_token_three, username_three, loc_three, NULL, NULL, NULL); 

	Session_Hashtable_Set(session_hashtable_outofrange, username_three, session_three);
	char* outofrange_str = Session_Hashtable_String(session_hashtable_outofrange);
	Log_log(log_config, LOG_DEBUG, "outofrange: %s\n", outofrange_str);

	unsigned int inrange_payload_body_size = 0;
	unsigned int inrange_num_items = 0;		
	Session_Hashtable_Calc_Size_Items(session_hashtable_inrange, &inrange_payload_body_size, &inrange_num_items);
	Log_log(log_config, LOG_DEBUG, "in range payload body size: %i inrange num_items:%i\n", inrange_payload_body_size, inrange_num_items);
	unsigned char* inrange_data = Session_Hashtable_To_Binary(session_hashtable_inrange, inrange_payload_body_size, inrange_num_items); 
		
	Log_log(log_config, LOG_DEBUG, "get binary form of out of range hashtable\n");
	unsigned int outofrange_payload_body_size = 0;
	unsigned int outofrange_num_items = 0;		
	Session_Hashtable_Calc_Size_Items(session_hashtable_outofrange, &outofrange_payload_body_size, &outofrange_num_items);
	Log_log(log_config, LOG_DEBUG, "out of range payload body size: %i out of range num_items:%i\n", outofrange_payload_body_size, outofrange_num_items);
	unsigned char* outofrange_data = Session_Hashtable_To_Binary(session_hashtable_outofrange, outofrange_payload_body_size, outofrange_num_items); 
	
	unsigned int body_size = 0;
	unsigned short num_items = 0;
	Binary_Unpack("!IH", outofrange_data, &body_size, &num_items); 

	//calc data sizes	
	unsigned int outofrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
	unsigned int outofrange_data_size = outofrange_header_size + outofrange_payload_body_size;
	unsigned int inrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
	unsigned int inrange_data_size = inrange_header_size + inrange_payload_body_size;

	Log_log(log_config, LOG_DEBUG, "inrange size: %i outofrange size: %i\n", inrange_data_size, outofrange_data_size);
	
	char* session_token = "ddsafndasnfsdjnferf934";
		
	//send in logged in response back to client
	unsigned char* data = Protocol_Login_Response(session_token, inrange_data_size, inrange_data, outofrange_data_size, outofrange_data);
	char* format = Protocol_Format_Get(data);
	Log_log(log_config, LOG_DEBUG, "format: %s\n", format);	
	unsigned int size_to_send = Binary_Calcsize(format);
	
	int test_size = Binary_Calcsize(HEADER_FORMAT) + inrange_data_size + outofrange_data_size;
	Log_log(log_config, LOG_DEBUG, "size to send:%i == size: %i\n", size_to_send, test_size);
	
	if(size_to_send == test_size) {
		Log_log(log_config, LOG_INFO, "test ping login multiple sessions response passed\n");
	}

}

void Test_Ping_Login_Response_Format_Multiple_Sessions_Two(LogConfig* log_config) {

	unsigned int initial_size = 20;
	
	//in range and out of range sessions that are logged in
	Session_Hashtable* session_hashtable_inrange = Session_Hashtable_Create(initial_size);
	Session_Hashtable* session_hashtable_outofrange = Session_Hashtable_Create(initial_size);
	
	char* session_token_one = "afjodjossdjoasi";
	char* username_one = "juxstapose";
	Location* loc_one = Location_Create(0, 0, "starting_zone", 15, 10);
	Session* session_one = Session_Create(session_token_one, username_one, loc_one, NULL, NULL, NULL); 

	char* session_token_two = "afjodkldfdjdsddjossdjoasi";
	char* username_two = "bob";
	Location* loc_two = Location_Create(0, 0, "starting_zone", 7, 3);
	Session* session_two = Session_Create(session_token_two, username_two, loc_two, NULL, NULL, NULL); 
	
	char* session_token_three = "afjodkldfedjoasi";
	char* username_three = "somedude";
	Location* loc_three = Location_Create(0, 0, "starting_zone", 30, 50);
	Session* session_three = Session_Create(session_token_three, username_three, loc_three, NULL, NULL, NULL); 
	
	char* session_token_four = "poopafjodkldfedjoasi";
	char* username_four = "wacom";
	Location* loc_four = Location_Create(0, 0, "starting_zone", 2000, 9000);
	Session* session_four = Session_Create(session_token_four, username_four, loc_four, NULL, NULL, NULL); 
	
	Session_Hashtable_Set(session_hashtable_inrange, username_one, session_one);
	Session_Hashtable_Set(session_hashtable_inrange, username_two, session_two);
	Session_Hashtable_Set(session_hashtable_inrange, username_three, session_three);
	Session_Hashtable_Set(session_hashtable_inrange, username_four, session_four);
	
	char* inrange_str = Session_Hashtable_String(session_hashtable_inrange);
	
	Log_log(log_config, LOG_DEBUG, "inrange: %s\n", inrange_str);

	char* session_token_five = "afjodksdfsdkfksdfldfdjdsddjossdjoasi";
	char* username_five = "joe";
	Location* loc_five = Location_Create(0, 0, "starting_zone", 27, 23);
	Session* session_five = Session_Create(session_token_five, username_five, loc_five, NULL, NULL, NULL); 
	
	char* session_token_six = "afjodkddssdjoasi";
	char* username_six = "gregorfty";
	Location* loc_six = Location_Create(0, 0, "starting_zone", 87, 44);
	Session* session_six = Session_Create(session_token_six, username_six, loc_six, NULL, NULL, NULL); 

	Session_Hashtable_Set(session_hashtable_outofrange, username_five, session_five);
	Session_Hashtable_Set(session_hashtable_outofrange, username_six, session_six);
	
	char* outofrange_str = Session_Hashtable_String(session_hashtable_outofrange);
	Log_log(log_config, LOG_DEBUG, "outofrange: %s\n", outofrange_str);

	unsigned int inrange_payload_body_size = 0;
	unsigned int inrange_num_items = 0;		
	Session_Hashtable_Calc_Size_Items(session_hashtable_inrange, &inrange_payload_body_size, &inrange_num_items);
	Log_log(log_config, LOG_DEBUG, "in range payload body size: %i inrange num_items:%i\n", inrange_payload_body_size, inrange_num_items);
	unsigned char* inrange_data = Session_Hashtable_To_Binary(session_hashtable_inrange, inrange_payload_body_size, inrange_num_items); 
		
	Log_log(log_config, LOG_DEBUG, "get binary form of out of range hashtable\n");
	unsigned int outofrange_payload_body_size = 0;
	unsigned int outofrange_num_items = 0;		
	Session_Hashtable_Calc_Size_Items(session_hashtable_outofrange, &outofrange_payload_body_size, &outofrange_num_items);
	Log_log(log_config, LOG_DEBUG, "out of range payload body size: %i out of range num_items:%i\n", outofrange_payload_body_size, outofrange_num_items);
	unsigned char* outofrange_data = Session_Hashtable_To_Binary(session_hashtable_outofrange, outofrange_payload_body_size, outofrange_num_items); 
	
	unsigned int body_size = 0;
	unsigned short num_items = 0;
	Binary_Unpack("!IH", outofrange_data, &body_size, &num_items); 

	//calc data sizes	
	unsigned int outofrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
	unsigned int outofrange_data_size = outofrange_header_size + outofrange_payload_body_size;
	unsigned int inrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
	unsigned int inrange_data_size = inrange_header_size + inrange_payload_body_size;

	Log_log(log_config, LOG_DEBUG, "inrange size: %i outofrange size: %i\n", inrange_data_size, outofrange_data_size);
	
	char* session_token = "ddsafndasnfsdjnferf934";
		
	//send in logged in response back to client
	unsigned char* data = Protocol_Login_Response(session_token, inrange_data_size, inrange_data, outofrange_data_size, outofrange_data);
	char* format = Protocol_Format_Get(data);
	Log_log(log_config, LOG_DEBUG, "format: %s\n", format);	
	unsigned int size_to_send = Binary_Calcsize(format);
	
	int test_size = Binary_Calcsize(HEADER_FORMAT) + inrange_data_size + outofrange_data_size;
	Log_log(log_config, LOG_DEBUG, "size to send:%i == size: %i\n", size_to_send, test_size);
	
	if(size_to_send == test_size) {
		Log_log(log_config, LOG_INFO, "test ping login multiple sessions response passed\n");
	}


}


int main(int argc, char* argv[]) {
	
	LogConfig* log_config = LogConfig_Create("log.txt", 
						 LOG_ON,
						 LOG_FILE,
						 LOG_DEBUG,
						 100000000, 
						 5);

	//Test_Ping_Login_Response_Format_No_Sessions(log_config);
	//Test_Ping_Login_Response_Format_Multiple_Sessions_One(log_config);
	Test_Ping_Login_Response_Format_Multiple_Sessions_Two(log_config);
	
	return 0;
}

