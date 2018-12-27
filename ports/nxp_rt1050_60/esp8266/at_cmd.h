/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _AT_CMD_H_
#define _AT_CMD_H_
#include "board.h"

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

void SysTick_Init(void);

int AT_CMD_AT_TSET(void);
int AT_CMD_ECHO(int open);
void AT_CMD_RESET(void);
int AT_CMD_UP_BAUDRATE(char *baudRate);
int AT_CMD_GET_WIFI_STATE(char *ssid_b);
int AT_CMD_DISCONNECT_AP(void);
int AT_CMD_SET_WIFI_MODE(char *mode);
int AT_CMD_CONNECT_AP(char *ssid, char *password);
int AT_CMD_GET_CONNECT_STATE(char (*status)[32]);
int AT_CMD_CONNECT_TCP(char *ip, char *port);
int	AT_CMD_TRANSIMITTAL_MODE(void);
int AT_CMD_AT_MODE(void);
int	AT_CMD_DISCONNECT_TCP(void);
int AT_CMD_DISCONNECT_AP(void);
int AT_CMD_SET_CIPMUX(void);
#endif
