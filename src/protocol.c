#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "pack.h"
#include "protocol.h"


char* Protocol_Get_Format(unsigned char* data) {

	char header_char_one;
	char header_char_two;
	char* session_token = (char*)malloc(SESSION_LENGTH + 1);
	memset(session_token, '\0', SESSION_LENGTH + 1);
	unsigned short cmd;
	unsigned short proto;
	unsigned int payload_size;
	
	Binary_Unpack(HEADER_FORMAT, data, &header_char_one, &header_char_two, session_token, &cmd, &proto, &payload_size);

	if(cmd == CMD_REGISTER && proto == PROTO_SEND) {
		//intialize string with header size
		int header_length = strlen(HEADER_FORMAT);
		//printf("header_length: %i\n", header_length);
		int new_format_length = header_length + 1 + 1;
		char* new_format = (char*)malloc(new_format_length);	
		int bytes = sprintf(new_format, "%sI", HEADER_FORMAT);
		//printf("%i == %i\n", strlen(new_format) +1, new_format_length);
		//printf("first format: %s\n", new_format);
		unsigned int username_size;
		Binary_Unpack(new_format, data, &header_char_one, &header_char_two, session_token, &cmd, &proto, &payload_size, &username_size);
		//printf("payload size: %i\n", payload_size);
		//printf("username size: %i\n", username_size);
			
		//find the length of username size as a string
		int username_length = Util_Count_Digits(username_size);
		int new_format_us_length = header_length + 1 + username_length + 1 + 1 + 1;
		char* new_format_us = (char*)malloc(new_format_us_length);
		bytes = sprintf(new_format_us, "%sI%isI", HEADER_FORMAT, username_size);
		//printf("%i == %i\n", strlen(new_format_us) +1, new_format_us_length);
		//printf("second format: %s\n", new_format_us);
		
		char* username = (char*)malloc(username_size+1);
		memset(username, '\0', username_size);
		unsigned int password_size;
		Binary_Unpack(new_format_us, data, &header_char_one, &header_char_two, session_token, &cmd, &proto, &payload_size, &username_size, username, &password_size);
		//printf("password size: %i\n", password_size);

		int password_length = Util_Count_Digits(password_size);	
		int new_format_ps_length = header_length + 1 + username_length + 1 + 1 + password_length + 1 + 1;
		char* new_format_ps = (char*)malloc(new_format_ps_length);	
		bytes = sprintf(new_format_ps, "%sI%isI%isI", HEADER_FORMAT, username_size, password_size);
		//printf("%i == %i\n", strlen(new_format_ps) +1, new_format_ps_length);
		//printf("result format: %s\n", new_format_ps);
		
		char* password = (char*)malloc(password_size+1);
		memset(password, '\0', password_size);
		unsigned int email_size;
		Binary_Unpack(new_format_ps, data, &header_char_one, &header_char_two, session_token, &cmd, &proto, &payload_size, 
				&username_size, username, &password_size, password, &email_size);
		
		int email_length = Util_Count_Digits(email_size);
		int new_format_es_length = header_length + 1 + username_length + 1 + 1 + password_length + 1 + 1 + email_length + 1 + 1;
		char* new_format_es = (char*)malloc(new_format_es_length);
		bytes = sprintf(new_format_es, "%sI%isI%isI%is", HEADER_FORMAT, username_size, password_size, email_size);
		
		free(new_format);	
		free(new_format_us);	
		free(new_format_ps);	
		free(username);
		return new_format_es;

	} else if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		int format_size = strlen(HEADER_FORMAT) + 1;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%s", HEADER_FORMAT);
		free(session_token);
		return format;
	} else if(cmd == CMD_LOGIN && proto == PROTO_SEND) {
		//printf("login extract format\n");
		//intialize string with header size
		int header_length = strlen(HEADER_FORMAT);
		//printf("header_length: %i\n", header_length);
		int new_format_length = header_length + 1 + 1;
		char* new_format = (char*)malloc(new_format_length);	
		int bytes = sprintf(new_format, "%sI", HEADER_FORMAT);
		//printf("%i == %i\n", strlen(new_format) +1, new_format_length);
		//printf("first format: %s\n", new_format);
		unsigned int username_size;
		Binary_Unpack(new_format, data, &header_char_one, &header_char_two, session_token, &cmd, &proto, &payload_size, &username_size);
		//printf("payload size: %i\n", payload_size);
		//printf("username size: %i\n", username_size);
			
		//find the length of username size as a string
		int username_length = Util_Count_Digits(username_size);
		int new_format_us_length = header_length + 1 + username_length + 1 + 1 + 1;
		char* new_format_us = (char*)malloc(new_format_us_length);
		bytes = sprintf(new_format_us, "%sI%isI", HEADER_FORMAT, username_size);
		//printf("%i == %i\n", strlen(new_format_us) +1, new_format_us_length);
		//printf("second format: %s\n", new_format_us);
		
		char* username = (char*)malloc(username_size+1);
		memset(username, '\0', username_size);
		unsigned int password_size;
		Binary_Unpack(new_format_us, data, &header_char_one, &header_char_two, session_token, &cmd, &proto, &payload_size, &username_size, username, &password_size);
		//printf("password size: %i\n", password_size);

		int password_length = Util_Count_Digits(password_size);	
		int new_format_ps_length = header_length + 1 + username_length + 1 + 1 + password_length + 1 + 1;
		char* new_format_ps = (char*)malloc(new_format_ps_length);	
		bytes = sprintf(new_format_ps, "%sI%isI%is", HEADER_FORMAT, username_size, password_size);
		//printf("%i == %i\n", strlen(new_format_ps) +1, new_format_ps_length);
		//printf("result format: %s\n", new_format_ps);
			
		free(new_format);	
		free(new_format_us);	
		free(username);
		return new_format_ps;

	}
	else if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		int format_size = strlen(HEADER_FORMAT) + 1;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%s", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else if(cmd == CMD_LOGOUT && proto == PROTO_SEND) {
		int format_size = strlen(HEADER_FORMAT) + 1;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%s", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else if(cmd == CMD_LOGOUT && proto == PROTO_RESPONSE) {
		int format_size = strlen(HEADER_FORMAT) + 1;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%s", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else if(cmd == CMD_MOVEMENT && proto == PROTO_SEND) {
		int format_size = strlen(HEADER_FORMAT) + 1 + 3;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%sHHH", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else if(cmd == CMD_MOVEMENT && proto == PROTO_BROADCAST) {
		int format_size = strlen(HEADER_FORMAT) + 1 + 3;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%sHHH", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else if(cmd == CMD_ERROR && proto == PROTO_RESPONSE) {
		int format_size = strlen(HEADER_FORMAT) + 1 + 1;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%sh", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else if(cmd == CMD_SHUTDOWN && proto == PROTO_SEND) {
		int format_size = strlen(HEADER_FORMAT) + 1;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%s", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else if(cmd == CMD_SHUTDOWN && proto == PROTO_RESPONSE) {
		int format_size = strlen(HEADER_FORMAT) + 1;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%s", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else if(cmd == CMD_PING && proto == PROTO_SEND) {
		int format_size = strlen(HEADER_FORMAT) + 1 +2;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%s", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else if(cmd == CMD_PING && proto == PROTO_RESPONSE) {
		int format_size = strlen(HEADER_FORMAT) + 1;
		char* format = (char*)malloc(format_size);
		memset(format, '\0', format_size);
		int bytes = sprintf(format, "%s", HEADER_FORMAT);
		free(session_token);
		return format;
	}
	else {
		free(session_token);
		return NULL;
	}
}

int Protocol_Is_Cmd_Valid(unsigned short cmd, unsigned short proto) {
	if( cmd >= 0 && cmd < NUM_CMD_MAX && proto >=0 && proto < NUM_PROTO_MAX) {
		return 1;
	}
	return 0;
}

unsigned char* Protocol_Register_Send(char* username, char* password, char* email) {
	char* session_token = (char*)malloc(SESSION_LENGTH+1);
	memset(session_token, '\0', SESSION_LENGTH+1);	
	memset(session_token, '0', SESSION_LENGTH);
	
	unsigned int username_size = strlen(username) + 1;
	unsigned int password_size = strlen(password) + 1;
	unsigned int email_size = strlen(email) + 1;		

	//printf("username size in pack: %i\n", username_size);
	//printf("password size in pack: %i\n", password_size);

	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%sI%isI%isI%is", HEADER_FORMAT, username_size, password_size, email_size);
	int payload_size = sizeof(unsigned int) + username_size + sizeof(unsigned int) + password_size + sizeof(unsigned int) + email_size;
	
	unsigned char* result = Binary_Pack(format, 'S','A', session_token, CMD_REGISTER, PROTO_SEND, payload_size, username_size, username, password_size, password, email_size, email);
	
	free(session_token);	
	return result;
}

unsigned char* Protocol_Register_Response() {
	char* session_token = (char*)malloc(SESSION_LENGTH+1);
	memset(session_token, '\0', SESSION_LENGTH+1);	
	memset(session_token, '0', SESSION_LENGTH);

	int payload_size = 0;
	
	unsigned char* result = Binary_Pack(HEADER_FORMAT, 'S','A', session_token, CMD_REGISTER, PROTO_RESPONSE, payload_size);
	
	free(session_token);	
	return result;
}

unsigned char* Protocol_Shutdown_Send() {
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	int payload_size = 0;
	char* session_token = (char*)malloc(SESSION_LENGTH+1);
	memset(session_token, '\0', SESSION_LENGTH+1);	
	memset(session_token, '0', SESSION_LENGTH);
	unsigned char* result = Binary_Pack(format, 'S', 'A',  session_token, CMD_SHUTDOWN, PROTO_SEND, payload_size);
	return result;
}

unsigned char* Protocol_Shutdown_Response() {
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	int payload_size = 0;
	char* session_token = (char*)malloc(SESSION_LENGTH+1);
	memset(session_token, '\0', SESSION_LENGTH+1);	
	memset(session_token, '0', SESSION_LENGTH);
	unsigned char* result = Binary_Pack(format, 'S', 'A',  session_token, CMD_SHUTDOWN, PROTO_RESPONSE, payload_size);
	return result;
}

unsigned char* Protocol_Login_Send(char* username, char* password) {
	
	//session token is generated at the server 
	//login cmd returns session token
	//so logins session token is empty
	
	char* session_token = (char*)malloc(SESSION_LENGTH+1);
	memset(session_token, '\0', SESSION_LENGTH+1);	
	memset(session_token, '0', SESSION_LENGTH);


	unsigned int username_size = strlen(username) + 1;
	unsigned int password_size = strlen(password) + 1;
	
	//printf("username size in pack: %i\n", username_size);
	//printf("password size in pack: %i\n", password_size);

	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%sI%isI%is", HEADER_FORMAT, username_size, password_size);
	int payload_size = sizeof(unsigned int) + username_size + sizeof(unsigned int) + password_size;
	
	//printf("%s\n", format);
	//printf("password to pack %s\n", password);
	unsigned char* result = Binary_Pack(format, 'S','A', session_token, CMD_LOGIN, PROTO_SEND, payload_size, username_size, username, password_size, password);
	
	
	free(session_token);	
	return result;
}

unsigned char* Protocol_Login_Response(char session_token[SESSION_LENGTH + 1],
		                       unsigned int inrange_size, unsigned char* inrange_data, 
				       unsigned int outofrange_size, unsigned char* outofrange_data) {
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	unsigned int header_size = Binary_Calcsize(format);
	
	int payload_size = inrange_size + outofrange_size;	

	unsigned char* header = Binary_Pack(format, 'S','A', session_token, CMD_LOGIN, PROTO_RESPONSE, payload_size);
	
	unsigned char* result = (unsigned char*)malloc(header_size + inrange_size + outofrange_size);
	memcpy(result, header, header_size);
	memcpy(result + header_size, inrange_data, inrange_size);
	memcpy(result + header_size + inrange_size, outofrange_data, outofrange_size);
	
	return result;	
}


unsigned char* Protocol_Movement_Send(char session_token[SESSION_LENGTH+1], 
		                      unsigned short direction, unsigned short speed, 
				      unsigned short previous_frames) {
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%sHHH", HEADER_FORMAT);
	int payload_size = sizeof(unsigned short) + sizeof(unsigned short) + sizeof(unsigned short);
	//printf("payload_size: %i\n", payload_size);	
	unsigned char* result = Binary_Pack(format, 'S','A', session_token, CMD_MOVEMENT, PROTO_SEND, payload_size, direction, speed, previous_frames);
	return result;	
}

unsigned char* Protocol_Movement_Response(char session_token[SESSION_LENGTH+1]) {
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	int payload_size = 0;
	unsigned char* result = Binary_Pack(format, 'S','A', session_token, CMD_MOVEMENT, PROTO_RESPONSE, payload_size);
	return result;	
}

unsigned char* Protocol_Movement_Broadcast(char session_token[SESSION_LENGTH+1], unsigned short direction, unsigned short speed, unsigned short frames) {
	
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%sHHH", HEADER_FORMAT);
	
	int payload_size = sizeof(unsigned short) + sizeof(unsigned short) + sizeof(unsigned short);
	//printf("payload_size: %i\n", payload_size);	

	unsigned char* result = Binary_Pack(format, 'S','A', session_token, CMD_MOVEMENT, PROTO_BROADCAST, payload_size, direction, speed, frames);
	return result;	
}

unsigned char* Protocol_Logout_Send(char session_token[SESSION_LENGTH+1]) {
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	int payload_size = 0;	
	unsigned char* result = Binary_Pack(format, 'S', 'A',  session_token, CMD_LOGOUT, PROTO_SEND, payload_size);
	return result;
}


unsigned char* Protocol_Logout_Response(char session_token[SESSION_LENGTH+1]) {
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	int payload_size = 0;	
	unsigned char* result = Binary_Pack(format, 'S', 'A',  session_token, CMD_LOGOUT, PROTO_RESPONSE, payload_size);
	return result;
}

unsigned char* Protocol_Ping_Send(char session_token[SESSION_LENGTH+1], int current_pos_x, int current_pos_y) {
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%sii", HEADER_FORMAT, current_pos_x, current_pos_y);
	int payload_size = sizeof(int) + sizeof(int);	
	unsigned char* result = Binary_Pack(format, 'S', 'A',  session_token, CMD_PING, PROTO_SEND, payload_size, current_pos_x, current_pos_y);
	return result;
}

unsigned char* Protocol_Ping_Response(char session_token[SESSION_LENGTH+1], 
		                      unsigned int inrange_size, unsigned char* inrange_data, 
				      unsigned int outofrange_size, unsigned char* outofrange_data) {
	
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	unsigned int header_size = Binary_Calcsize(format);
	
	int payload_size = inrange_size + outofrange_size;	
	unsigned char* header = Binary_Pack(format, 'S', 'A',  session_token, CMD_PING, PROTO_RESPONSE, payload_size);
	
	unsigned char* result = (unsigned char*)malloc(header_size + inrange_size + outofrange_size);
	memcpy(result, header, header_size);
	memcpy(result + header_size, inrange_data, inrange_size);
	memcpy(result + header_size + inrange_size, outofrange_data, outofrange_size);

	return result;
}

unsigned char* Protocol_Error_Response(char session_token[SESSION_LENGTH+1], unsigned short error_code) {
	
	char* input_session = (char*)malloc(SESSION_LENGTH+1);
	memset(input_session, '\0', SESSION_LENGTH+1);
	memset(input_session, '0', SESSION_LENGTH);

	if(session_token != NULL) {
		memcpy(input_session, session_token, SESSION_LENGTH+1);
	}
	
	char format[128] = {'\0'};	
	int bytes = sprintf(format, "%sH", HEADER_FORMAT);
	int payload_size = sizeof(unsigned short);
	unsigned char* result = Binary_Pack(format, 'S', 'A',  input_session, CMD_ERROR, PROTO_RESPONSE, payload_size, error_code);
	
	return result;
}




void Protocol_Register_Send_Payload_Unpack(unsigned char* payload, char username_out[USERNAME_LENGTH +1], 
								   char password_out[PASSWORD_LENGTH+1], 
								   char email_out[EMAIL_LENGTH+1]) {
	if(payload != NULL) {
		int nbo = 1;
		int i_char = 1;
		int end_byte = 1;
		int string_char = 1;

		char* new_format_one = (char*)malloc(nbo + i_char + end_byte);	
		int bytes = sprintf(new_format_one, "!I");
		unsigned int username_size;
		Binary_Unpack(new_format_one, payload, &username_size);
		free(new_format_one);

		unsigned int un_length = Util_Count_Digits(username_size);
		char* new_format_two = (char*)malloc(nbo + i_char + un_length + string_char + i_char + end_byte);
		bytes = sprintf(new_format_two, "!I%isI", username_size);
		unsigned int password_size;
		Binary_Unpack(new_format_two, payload, &username_size, username_out, &password_size);
		free(new_format_two);
			
		unsigned int pw_length = Util_Count_Digits(password_size);
		char* new_format_three = (char*)malloc(nbo + i_char + un_length + string_char + i_char + pw_length + string_char + i_char + end_byte);
		bytes = sprintf(new_format_three, "!I%isI%isI", username_size, password_size);
		unsigned int email_size;
		Binary_Unpack(new_format_three, payload, &username_size, username_out, &password_size, password_out, &email_size);
		free(new_format_three);
		
			
		unsigned int em_length = Util_Count_Digits(email_size);
		char* new_format_four = (char*)malloc(nbo + i_char + un_length + string_char + i_char + pw_length + string_char + i_char + em_length + string_char + end_byte);
		bytes = sprintf(new_format_four, "!I%isI%isI%is", username_size, password_size, email_size);
		Binary_Unpack(new_format_four, payload, &username_size, username_out, &password_size, password_out, &email_size, email_out);
		free(new_format_four);
	}

}

void Protocol_Login_Send_Payload_Unpack(unsigned char* payload, char username_out[USERNAME_LENGTH+1], char password_out[PASSWORD_LENGTH+1]) {
	if(payload != NULL) {
		int nbo = 1;
		int i_char = 1;
		int end_byte = 1;
		int string_char = 1;

		char* new_format_one = (char*)malloc(nbo + i_char + end_byte);	
		int bytes = sprintf(new_format_one, "!I");
		unsigned int username_size;
		Binary_Unpack(new_format_one, payload, &username_size);
		free(new_format_one);

		unsigned int un_length = Util_Count_Digits(username_size);
		char* new_format_two = (char*)malloc(nbo + i_char + un_length + string_char + i_char + end_byte);
		bytes = sprintf(new_format_two, "!I%isI", username_size);
		unsigned int password_size;
		Binary_Unpack(new_format_two, payload, &username_size, username_out, &password_size);
		free(new_format_two);
			
		unsigned int pw_length = Util_Count_Digits(password_size);
		char* new_format_three = (char*)malloc(nbo + i_char + un_length + string_char + i_char + pw_length + string_char + i_char + end_byte);
		bytes = sprintf(new_format_three, "!I%isI%is", username_size, password_size);
		Binary_Unpack(new_format_three, payload, &username_size, username_out, &password_size, password_out);
		free(new_format_three);
	}
}

void Protocol_Ping_Send_Payload_Unpack(unsigned char* payload, int *current_pos_x, int *current_pos_y) {
	Binary_Unpack("!ii", payload, current_pos_x, current_pos_y);
}

void Protocol_Movement_Send_Payload_Unpack(unsigned char* payload, unsigned short *direction, unsigned short *speed, unsigned short *previous_frames) {
	Binary_Unpack("!HHH", payload, direction, speed, previous_frames);
}


void Protocol_Error_Response_Payload_Unpack(unsigned char* payload, unsigned short *error_code) {
	Binary_Unpack("!H", payload, error_code);
}

void Protocol_Header_Unpack(unsigned char* data, unsigned short* cmd, unsigned short* proto, unsigned int* payload_size) {
	char header_char_one;
	char header_char_two;
	char* session_token = (char*)malloc(SESSION_LENGTH + 1);
	memset(session_token, '\0', SESSION_LENGTH + 1);
	Binary_Unpack(HEADER_FORMAT, data, &header_char_one, &header_char_two, session_token, cmd, proto, payload_size);
}

void Protocol_Session_Unpack(unsigned char* data, char* session_token) {
	char header_char_one;
	char header_char_two;
	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;
	Binary_Unpack(HEADER_FORMAT, data, &header_char_one, &header_char_two, session_token, &cmd, &proto, &payload_size);
}
