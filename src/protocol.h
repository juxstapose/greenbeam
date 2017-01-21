#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define CMD_LOGIN 0
#define CMD_MOVEMENT 1
#define CMD_LOGOUT 2
#define CMD_REGISTER 3
#define CMD_ERROR 4
#define CMD_PING 5
#define CMD_SHUTDOWN 6

#define PROTO_SEND 0
#define PROTO_RESPONSE 1
#define PROTO_BROADCAST 2

#define NUM_CMD_MAX 6
#define NUM_PROTO_MAX 3

#define ERR_REG 0
#define ERR_LOGIN_NOT_REG 1
#define ERR_LOGIN_AGAIN 2
#define ERR_SESSION_NO_EXIST 3

#define HEADER_FORMAT "!2b65sHHI"
#define FORMAT_LENGTH 128

#define SESSION_LENGTH 64
#define USERNAME_LENGTH 32
#define PASSWORD_LENGTH 64
#define EMAIL_LENGTH 254

char* Protocol_Get_Format(unsigned char* data);
int Protocol_Is_Cmd_Valid(unsigned short cmd, unsigned short proto);
unsigned char* Protocol_Register_Send(char* username, char* password, char* email);
unsigned char* Protocol_Register_Response();
unsigned char* Protocol_Login_Send(char username[USERNAME_LENGTH+1], char password[PASSWORD_LENGTH+1]);
unsigned char* Protocol_Login_Response(char session_token[SESSION_LENGTH + 1]);
unsigned char* Protocol_Movement_Send(char session_token[SESSION_LENGTH+1], unsigned short direction, unsigned short speed, unsigned short previous_frames);
unsigned char* Protocol_Movement_Response(char session_token[SESSION_LENGTH+1]);
unsigned char* Protocol_Movement_Broadcast(char session_token[SESSION_LENGTH+1], unsigned short direction, unsigned short speed);
unsigned char* Protocol_Logout_Send(char session_token[SESSION_LENGTH+1]);
unsigned char* Protocol_Logout_Response(char session_token[SESSION_LENGTH+1]);
unsigned char* Protocol_Ping_Send(char session_token[SESSION_LENGTH+1]);
unsigned char* Protocol_Ping_Response(char session_token[SESSION_LENGTH+1]);
unsigned char* Protocol_Error_Response(char session_token[SESSION_LENGTH+1], unsigned short error_code);
unsigned char* Protocol_Shutdown_Send();
unsigned char* Protocol_Shutdown_Response();
void Protocol_Register_Send_Payload_Unpack(unsigned char* payload, char username_out[USERNAME_LENGTH +1], 
								   char password_out[PASSWORD_LENGTH+1], 
								   char email_out[EMAIL_LENGTH+1]);
void Protocol_Login_Send_Payload_Unpack(unsigned char* payload, char username_out[USERNAME_LENGTH+1], 
								char password_out[PASSWORD_LENGTH+1]);
void Protocol_Movement_Send_Payload_Unpack(unsigned char* payload, unsigned short *direction, unsigned short *speed, unsigned short *previous_frames);
void Protocol_Error_Response_Payload_Unpack(unsigned char* payload, unsigned short *error_code);
void Protocol_Ping_Send_Payload_Unpack(unsigned char* payload, int *current_pos_x, int *current_pos_y);
void Protocol_Header_Unpack(unsigned char* data, unsigned short* cmd, unsigned short* proto, unsigned int* payload_size);
void Protocol_Session_Unpack(unsigned char* data, char* session_token);

#endif
