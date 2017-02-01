#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "pack.h"
#include "protocol.h"
#include "protocol_format.h"

char** Protocol_Format_Get_Table(unsigned char* data, unsigned int *body_header_size, unsigned int *format_size, unsigned int num_items) {
	char** result = (char**)calloc(num_items, FORMAT_LENGTH);
	
	*body_header_size = sizeof(unsigned int) + sizeof(short);
	
	unsigned char* item_data = data + *body_header_size;
	int x = 0;
	*format_size = 0;
	for(x=0; x<num_items; x++) {
		unsigned int username_size = 0;
		Binary_Unpack("!I", item_data, &username_size); 
		result[x] = (char*)calloc(1, FORMAT_LENGTH);
		int bytes = sprintf(result[x], "I%isii", username_size);
		*format_size += bytes;
		unsigned int item_size = Binary_Calcsize(result[x]);
		item_data = item_data + item_size;
	}	
	return result;
}

char* Protocol_Format_Build(char** format_table, unsigned int header_format_size, unsigned int format_size, unsigned int num_items) {
	char* format = (char*)malloc(header_format_size + format_size + 1);
	memset(format, '\0', header_format_size + format_size);		
	unsigned int total_bytes = 0;
	int header_bytes = sprintf(format, "IH");
	format += header_bytes;
	total_bytes += header_bytes;
	int x = 0;
	for(x=0; x<num_items; x++) {
		int format_bytes = sprintf(format, "%s", format_table[x]); 
		format += format_bytes;
		total_bytes += format_bytes;
	}
	format -= total_bytes;
	return format;
}

void Protocol_Format_Get_Header_Items(unsigned char* data, unsigned int *body_header_format_size, unsigned int *size, unsigned short *num_items) {

	*body_header_format_size = 2;
	unsigned int network_symbol = 1;
	unsigned int header_format_size = *body_header_format_size + network_symbol;
	char* header_format = (char*)malloc(header_format_size + 1);
	memset(header_format, '\0', header_format_size + 1);
	int header_bytes = sprintf(header_format, "!IH");
	Binary_Unpack(header_format, data, size, num_items); 
	
	free(header_format);		

}

char* Protocol_Format_Login_Send(unsigned char* data) {
	//printf("login extract format\n");
	//intialize string with header size
	int header_length = strlen(HEADER_FORMAT);
	//printf("header_length: %i\n", header_length);
	int new_format_length = header_length + 1 + 1;
	char* new_format = (char*)malloc(new_format_length);	
	int bytes = sprintf(new_format, "%sI", HEADER_FORMAT);
	//printf("%i == %i\n", strlen(new_format) +1, new_format_length);
	//printf("first format: %s\n", new_format);
	char header_char_one;
	char header_char_two;
	char session_token[SESSION_LENGTH+1] = {'\0'};
	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;
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


char* Protocol_Format_Login_Response(unsigned char* data) {
	
	unsigned int header_size = Binary_Calcsize(HEADER_FORMAT);
	
	unsigned int inrange_header_format_size = 0;
	unsigned int inrange_body_size = 0;	
	unsigned short inrange_num_items = 0;
	unsigned char* inrange_data = data + header_size;	
	Protocol_Format_Get_Header_Items(inrange_data, &inrange_header_format_size, &inrange_body_size, &inrange_num_items);
	
	unsigned int inrange_format_size = 0;
	unsigned int inrange_body_header_size = 0;	
	char** inrange_format_table = Protocol_Format_Get_Table(inrange_data, &inrange_body_header_size, &inrange_format_size, inrange_num_items);
	char* inrange_format = Protocol_Format_Build(inrange_format_table, inrange_header_format_size, inrange_format_size, inrange_num_items);

	unsigned int outofrange_header_format_size = 0;
	unsigned int outofrange_body_size = 0;	
	unsigned short outofrange_num_items = 0;
	
	unsigned char* outofrange_data = data + header_size + inrange_body_header_size + inrange_body_size;	
	Protocol_Format_Get_Header_Items(outofrange_data, &outofrange_header_format_size, &outofrange_body_size, &outofrange_num_items);
	
	unsigned int outofrange_format_size = 0;
	unsigned int outofrange_body_header_size = 0;	
	char** outofrange_format_table = Protocol_Format_Get_Table(outofrange_data, &outofrange_body_header_size, &outofrange_format_size, outofrange_num_items);
	char* outofrange_format = Protocol_Format_Build(outofrange_format_table, outofrange_header_format_size, outofrange_format_size, outofrange_num_items);
	
	unsigned int format_size = header_size + strlen(inrange_format) + strlen(outofrange_format) + 1;
	printf("format size: %i\n", format_size);	
	char* format = (char*)malloc(format_size);	
	memset(format, '\0', format_size);
	memcpy(format, HEADER_FORMAT, strlen(HEADER_FORMAT));
	memcpy(format + strlen(HEADER_FORMAT), inrange_format, strlen(inrange_format));
	memcpy(format + strlen(HEADER_FORMAT) + strlen(inrange_format), outofrange_format, strlen(outofrange_format));
	printf("format: %s\n", format);
	return format;
}

char* Protocol_Format_Register_Send(unsigned char* data) {
	
	//intialize string with header size
	int header_length = strlen(HEADER_FORMAT);
	//printf("header_length: %i\n", header_length);
	int new_format_length = header_length + 1 + 1;
	char* new_format = (char*)malloc(new_format_length);	
	int bytes = sprintf(new_format, "%sI", HEADER_FORMAT);
	//printf("%i == %i\n", strlen(new_format) +1, new_format_length);
	//printf("first format: %s\n", new_format);
	char header_char_one;
	char header_char_two;
	char session_token[SESSION_LENGTH+1] = {'\0'};
	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;
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
}

char* Protocol_Format_Register_Response(unsigned char* data) {

	int format_size = strlen(HEADER_FORMAT) + 1;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	return format;


}

char* Protocol_Format_Logout_Send(unsigned char* data) {

	int format_size = strlen(HEADER_FORMAT) + 1;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	return format;

}
char* Protocol_Format_Logout_Response(unsigned char* data) {

	int format_size = strlen(HEADER_FORMAT) + 1;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	return format;

}

char* Protocol_Format_Movement_Send(unsigned char* data) {
	int format_size = strlen(HEADER_FORMAT) + 3 + 1;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%sHHH", HEADER_FORMAT);
	return format;

}

char* Protocol_Format_Movement_Broadcast(unsigned char* data) {
	int format_size = strlen(HEADER_FORMAT) + 1 + 3;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%sHHH", HEADER_FORMAT);
	return format;
}

char* Protocol_Format_Movement_Response(unsigned char* data) {
	int format_size = strlen(HEADER_FORMAT) + 1;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	return format;
}

char* Protocol_Format_Error_Response(unsigned char* data) {
	int format_size = strlen(HEADER_FORMAT) + 1 + 1;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%sh", HEADER_FORMAT);
	return format;
}

char* Protocol_Format_Ping_Send(unsigned char* data) {
	int format_size = strlen(HEADER_FORMAT) + 2 + 1;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%sii", HEADER_FORMAT);
	return format;
}

char* Protocol_Format_Ping_Response(unsigned char* data) {

	return Protocol_Format_Login_Response(data);
}

char* Protocol_Format_Shutdown_Send(unsigned char* data) {

	int format_size = strlen(HEADER_FORMAT) + 1;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	return format;

}

char* Protocol_Format_Shutdown_Response(unsigned char* data) {
	int format_size = strlen(HEADER_FORMAT) + 1;
	char* format = (char*)malloc(format_size);
	memset(format, '\0', format_size);
	int bytes = sprintf(format, "%s", HEADER_FORMAT);
	return format;
}

char* Protocol_Format_Get(unsigned char* data) {

	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);
	
	return FormatCmd_Dispatch(data, cmd, proto);
}

FormatCmd* FormatCmd_Create() {
	FormatCmd* format_cmd = (FormatCmd*)malloc(sizeof(FormatCmd));
	int x = 0;
	int y = 0;
	for(x=0; x< NUM_CMD_MAX; x++) {
		for(y=0; y<NUM_PROTO_MAX; y++) {
			format_cmd->format_func_table[x][y] = NULL;
		}	
	}
	format_cmd->format_func_table[CMD_REGISTER][PROTO_SEND] = Protocol_Format_Register_Send;
	format_cmd->format_func_table[CMD_REGISTER][PROTO_RESPONSE] = Protocol_Format_Register_Response;
	format_cmd->format_func_table[CMD_LOGIN][PROTO_SEND] = Protocol_Format_Login_Send;
	format_cmd->format_func_table[CMD_LOGIN][PROTO_RESPONSE] = Protocol_Format_Login_Response;
	format_cmd->format_func_table[CMD_LOGOUT][PROTO_SEND] = Protocol_Format_Logout_Response;
	format_cmd->format_func_table[CMD_LOGOUT][PROTO_RESPONSE] = Protocol_Format_Logout_Response;
	format_cmd->format_func_table[CMD_MOVEMENT][PROTO_SEND] = Protocol_Format_Movement_Send;
	format_cmd->format_func_table[CMD_MOVEMENT][PROTO_BROADCAST] = Protocol_Format_Movement_Broadcast;
	format_cmd->format_func_table[CMD_MOVEMENT][PROTO_RESPONSE] = Protocol_Format_Movement_Response;
	format_cmd->format_func_table[CMD_PING][PROTO_SEND] = Protocol_Format_Ping_Send;
	format_cmd->format_func_table[CMD_PING][PROTO_RESPONSE] = Protocol_Format_Ping_Response;
	format_cmd->format_func_table[CMD_SHUTDOWN][PROTO_SEND] = Protocol_Format_Shutdown_Send;
	format_cmd->format_func_table[CMD_SHUTDOWN][PROTO_RESPONSE] = Protocol_Format_Shutdown_Response;
	format_cmd->format_func_table[CMD_ERROR][PROTO_RESPONSE] = Protocol_Format_Error_Response;
	return format_cmd;
}

void FormatCmd_Destroy(FormatCmd* format_cmd) {
	if(format_cmd != NULL) {
		free(format_cmd);
	}
}

char* FormatCmd_Dispatch(unsigned char* data, unsigned short cmd, unsigned short proto) {
	int cmd_valid = Protocol_Is_Cmd_Valid(cmd, proto);
	FormatCmd* format_cmd = FormatCmd_Create();
	if(cmd_valid) { 
		if(format_cmd->format_func_table[cmd][proto] != NULL) {
			char* result = (*format_cmd->format_func_table[cmd][proto])(data);
			//Log_log(log_config, LOG_INFO, "cmd:%i proto:%i format:%s\n", cmd, proto, result);
			FormatCmd_Destroy(format_cmd);
			return result;
		}
	} else {
		//Log_log(log_config, LOG_INFO, "cmd and proto not valid: cmd:%i proto:%i\n", cmd, proto);
	}
	FormatCmd_Destroy(format_cmd);
	return NULL;
}
