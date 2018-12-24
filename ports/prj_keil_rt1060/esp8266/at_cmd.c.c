/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "esp8266.h"
#include "core_cm7.h"
#include <string.h>
#include <stdint.h>
#include "at_cmd.h"
#include "board.h"
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#define MAX_RESULT_LINE   8
#define MAX_RESULT_NUM    128
#define MAX_COMMAND_NUM    64
#define SYSCLK   600
/*******************************************************************************
 * Variables
 ******************************************************************************/

//void SysTick_Init()
//{							   		
//	SysTick->CTRL|=SysTick_CTRL_CLKSOURCE_Msk;
//	SysTick->LOAD=16777215;
//	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
//}	

//void delay_us(uint32_t nus)
//{		
//	uint32_t ticks;
//	uint32_t told,tnow,tcnt=0;
//	uint32_t reload=SysTick->LOAD;				//LOAD的值	    	 
//	ticks=nus*SYSCLK; 						//需要的节拍数 
//	told=SysTick->VAL;        				//刚进入时的计数器值
//	while(1)
//	{
//		tnow=SysTick->VAL;	
//		if(tnow!=told)
//		{	    
//			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
//			else tcnt+=reload-tnow+told;	    
//			told=tnow;
//			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
//		}  
//	};
//}
  
//延时nms
//nms:要延时的ms数
void delay_ms(uint16_t nms)
{
//	uint32_t i;
//	for(i=0;i<nms;i++) delay_us(1000);
	HAL_Delay(nms);

}

static int str_to_line(char *cmd_res, size_t len, size_t *offsets, size_t *o_len)
{
	size_t i;
  int state = 0; 
	size_t e_index = 0;    // Ending flag starting 
	size_t o_index = 0;   

	for (i=0; (i< len) && (o_index < *o_len) ; i++){
		switch (state) {
			case 0: 
				if (cmd_res[i] != 0x0D){
					offsets[o_index++] = i;	
				  state = 1;					
				} else {
					e_index = i;
					state = 2;
				}
				break;
		  case 1: 
				if (cmd_res[i] == 0x0D){
						e_index = i;
						state = 2;
				}
				break;
			case 2: 
				if (cmd_res[i] == 0x0A){
						state = 0;
					  cmd_res[e_index]='\0';
				}
				else if (cmd_res[i] == 0x0D){
						state = 2;
				}
				else {
						state = 1;
				}
				break;
			default:
				break;
		}
		 
	}
	*o_len = o_index;
	return  state ? 1: 0;
}

//void at_callback(status_t status, void *userData)
//{
//		if (kStatus_LPUART_TxIdle == status)
//    {
//					esp8266_at_recv(*((char **)userData), MAX_RESULT_NUM);
//    }

//    if (kStatus_LPUART_RxIdle == status)
//    {
//        
//    }
//}

/*!
 * @brief AT CMD function
 */
static int AT_CMD(char *command, char *result, size_t *offsets, size_t *offset_len)
{
	  size_t len = strlen(command);
	  size_t recv_len = MAX_RESULT_NUM;
	  int i;
	 	char cmd[MAX_COMMAND_NUM];
		int j = 0;
		assert(len < MAX_COMMAND_NUM-3);
	  size_t o_len = *offset_len;
	  strcpy(cmd, command);
	  cmd[len] = 0x0D;
		cmd[len + 1] = 0x0A;
	  esp8266_at_send(cmd, len+2);
	  len = esp8266_at_recv(result, recv_len);
    while (1) {
			if ( 200 < j++)
			{
				return 1;
			}
			delay_ms(100);
	    len += esp8266_at_abort_recv();
			o_len = *offset_len;
			if (!str_to_line(result, len, offsets, &o_len)){  // The result is end with "\r\n"
					if (o_len) {						
						for (i = 0; i < o_len; i++){
							if (!strcmp(&result[offsets[i]], "OK")){
								*offset_len = o_len;	
								return 0;
							}
							if (!strcmp(&result[offsets[i]], "ERROR")){
								*offset_len = o_len;
								return 1;	
							}
							if (!strcmp(&result[offsets[i]], "FAIL")) {
								*offset_len = o_len;
								return 1;								
							}			
						}			
					}			
			}
			else if (result[len - 1] == '>') {
					return 0;
			}			
			len += esp8266_at_recv(result + len, recv_len - len);    
		}
}

/*!
 * @brief AT_TEST function
 */
int AT_CMD_AT_TSET()
{	
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	return AT_CMD("AT", result, offsets, &offset_len);	  
}

void AT_CMD_RESET()
{
	char cmd[12] = "AT+RST";
	size_t len = strlen(cmd);
	cmd[len] = 0x0D;
	cmd[len + 1] = 0x0A;	
	esp8266_at_stop_ring();
	esp8266_at_send(cmd, len+2);
	delay_ms(500);
//	esp8266_at_start_ring();
}
/*!
 * @brief AT_TEST function
 */

int AT_CMD_ECHO(int open) 
{	
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	if (open){
		return AT_CMD("ATE1", result, offsets, &offset_len);
	}
	else{
    return AT_CMD("ATE0", result, offsets, &offset_len);
	}		
}

int AT_CMD_UP_BAUDRATE(char *baudRate)
{
	char cmd[64] = "AT+UART_CUR=";
	size_t len = strlen(cmd);
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	
  strcpy(cmd + len, baudRate); 
	len = strlen(cmd);
	strcpy(cmd + len, ",8,1,0,0");
	return AT_CMD(cmd, result, offsets, &offset_len);	  

}

int AT_CMD_GET_WIFI_STATE(char *ssid_b)
{
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	int i;
	char *ssid_s, *ssid_e;
	
	if( !AT_CMD("AT+CWJAP_CUR?", result, offsets, &offset_len))	{
		for (i = 0; i < offset_len; i++){
			if (!strncmp(&result[offsets[i]], "+CWJAP_CUR:",11)){					
				if ((ssid_s = strchr(&result[offsets[i]],'"')) != NULL)
					if ((ssid_e = strchr(ssid_s+1,'"')) != NULL)
						 memcpy(ssid_b, ssid_s, ssid_e- ssid_s);
					   ssid_b[ssid_e - ssid_s + 1] = '\0';
						 return 0;
			}
		}
	}
	return 1;
}
int AT_CMD_DISCONNECT_AP()
{
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	return AT_CMD("AT+CWQAP", result, offsets, &offset_len);
}

int AT_CMD_SET_WIFI_MODE(char *mode)
{
	char cmd[64] = "AT+CWMODE_CUR=";
	size_t len = strlen(cmd);
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	
  strcpy(cmd + len, mode); 
	return AT_CMD(cmd, result, offsets, &offset_len);	 
}

int AT_CMD_CONNECT_AP(char *ssid, char *password)
{
	char cmd[128] = "AT+CWJAP_CUR=";
	size_t len = strlen(cmd);
	char result[MAX_RESULT_NUM] = {0};
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	
  cmd[len] = '"'; 
	strcpy(cmd + len + 1, ssid); 
	len = strlen(cmd);
	strcpy(cmd + len , "\",\"");
	len += 3;
	strcpy(cmd + len, password);
	len = strlen(cmd);
	cmd[len] = '"'; 
	cmd[len + 1] = '\0'; 
	return AT_CMD(cmd, result, offsets, &offset_len);
}

int AT_CMD_GET_CONNECT_STATE(char (*status)[32])
{
	char cmd[64] = "AT+CIPSTATUS";
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
  int i = 0;
	char *s, *e;
	if (!AT_CMD(cmd, result, offsets, &offset_len)){
		for ( i = 0; i < offset_len; i++){
			if (!strncmp(&result[offsets[i]], "STATUS:", 7)) {
				if ((s=strchr(&result[offsets[i]], ':' ))!= NULL){
				   status[0][0] = *(s + 1);
					 status[0][1] = '\0';
					 break;
				} else {
					 return 1;
				}
			}
		}
		if (i >= offset_len)
				return 1;
		if (status[0][0] == '3') { 
			  if ((s = strchr(&result[offsets[i + 1]], '"' )) != NULL){
					if ((s = strchr(s + 1, '"' ))!= NULL){
						 if ((s = strchr(s + 1, '"' ))!= NULL){
						     if ((e = strchr(s + 1, '"' ))!= NULL){
									 memcpy(status[1], s, e - s);
									 status[1][e -s + 1] = '\0';
									 s = e + 2;
									 if ((e = strchr(s + 1, ',' ))!= NULL){
											memcpy(status[2], s, e - s);
										  status[2][e -s + 1] = '\0';
										  return 0;
									 }
								 }
						 }
					}
				}
			status[1][0] = '\0';
			status[2][0] = '\0';
		 } 	
	}
	return 0;
}

int AT_CMD_CONNECT_TCP(char *ip, char *port)
{
		char cmd[128] = "AT+CIPSTART=\"TCP\",\"";
	size_t len = strlen(cmd);
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	
	strcpy(cmd + len , ip); 
	len = strlen(cmd);
	strcpy(cmd + len , "\",");
	len += 2;
	strcpy(cmd + len, port);
//	len = strlen(cmd);
//	strcpy(cmd + len , ",1");
//	cmd[len + 2] = '\0';
	return AT_CMD(cmd, result, offsets, &offset_len);
}

int	AT_CMD_TRANSIMITTAL_MODE()
{
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	if (!AT_CMD("AT+CIPMODE=1", result, offsets, &offset_len))
	 return AT_CMD("AT+CIPSEND", result, offsets, &offset_len);
  return 1;	
}

int AT_CMD_AT_MODE()
{
//	uint8_t buf[2] = {0x0D, 0x0D, 0x0A};
	esp8266_at_send("+++", 3);
//	esp8266_at_send(buf, 2);
	delay_ms(500);
  return 0;
}

int	AT_CMD_DISCONNECT_TCP()
{
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	return AT_CMD("AT+CIPCLOSE", result, offsets, &offset_len);
}

int AT_CMD_SET_CIPMUX()
{
	char result[MAX_RESULT_NUM];
	size_t offsets[MAX_RESULT_LINE];
	size_t offset_len = MAX_RESULT_LINE;
	return AT_CMD("AT+CIPMUX=0", result, offsets, &offset_len);
}
