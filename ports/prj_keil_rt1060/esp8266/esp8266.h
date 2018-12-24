/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ESP8266_H_
#define _ESP8266_H_
#include "board.h"
#include "at_cmd.h"
#include "fsl_lpuart.h"

#define RxIdle (kStatus_LPUART_RxIdle)
#define TxIdle (kStatus_LPUART_TxIdle)

typedef  void (*user_handle_t)(status_t status, void *userData);
struct esp_Data_t
{
	  user_handle_t user_handle;
	  void  *userData;
};
int esp8266_uart_init(struct esp_Data_t *esp_Data);
int esp8266_wifi_setup(char *ssid, char *password);
int esp8266_create_tcp_client(char *ip, char *port);
void esp8266_send(void *buf, size_t size);
void esp8266_recv(void *buf, size_t size);
void esp8266_abort_recv(void); 
void esp8266_at_send(void *buf, size_t size);
size_t esp8266_at_recv(void *buf, size_t size);
size_t esp8266_at_abort_recv(void);

void esp8266_at_stop_ring(void);

void esp8266_at_start_ring(void);
void esp8266_disconnect_client(void);

void esp8266_disconnect_wifi(void);
void esp8266_reset(void);
#endif
