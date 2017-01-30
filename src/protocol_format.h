#ifndef _PROTOCOL_FORMAT_H_
#define _PROTOCOL_FORMAT_H_

#define HEADER_FORMAT "!2b65sHHI"
#define FORMAT_LENGTH 512

#include "protocol.h"
#include "log.h"

char** Protocol_Format_Get_Table(unsigned char* data, unsigned int *body_header_size, unsigned int *format_size, unsigned int num_items);
char* Protocol_Format_Build(char** format_table, unsigned int header_format_size, unsigned int format_size, unsigned int num_items);
void Protocol_Format_Get_Header_Items(unsigned char* data, unsigned int *body_header_format_size, unsigned int *size, unsigned short *num_items);

typedef char* (*format_func)(unsigned char*);

typedef struct FormatCmd {
	format_func format_func_table[NUM_CMD_MAX][NUM_PROTO_MAX];
} FormatCmd;

FormatCmd* FormatCmd_Create();
void FormatCmd_Destroy(FormatCmd* format_cmd);
char* FormatCmd_Dispatch(unsigned char* data, unsigned short cmd, unsigned short proto);

char* Protocol_Format_Register_Send(unsigned char* data);
char* Protocol_Format_Register_Response(unsigned char* data);
char* Protocol_Format_Login_Send(unsigned char* data);
char* Protocol_Format_Login_Response(unsigned char* data);
char* Protocol_Format_Logout_Send(unsigned char* data);
char* Protocol_Format_Logout_Response(unsigned char* data);
char* Protocol_Format_Movement_Send(unsigned char* data);
char* Protocol_Format_Movement_Broadcast(unsigned char* data);
char* Protocol_Format_Movement_Response(unsigned char* data);
char* Protocol_Format_Error_Response(unsigned char* data);
char* Protocol_Format_Ping_Send(unsigned char* data);
char* Protocol_Format_Ping_Response(unsigned char* data);
char* Protocol_Format_Shutdown_Send(unsigned char* data);
char* Protocol_Format_Shutdown_Response(unsigned char* data);


char* Protocol_Format_Get(unsigned char* data);


#endif
