/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_lpuart_edma.h"
#include "fsl_lpuart.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_lpuart_edma.h"
#include "esp8266.h"
#include "at_cmd.h"
#include "fsl_iomuxc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define AT_LPUART LPUART4
#define AT_LPUART_CLK_FREQ BOARD_DebugConsoleSrcFreq()
#define LPUART_TX_DMA_CHANNEL 0U
#define LPUART_RX_DMA_CHANNEL 1U
#define LPUART_TX_DMA_REQUEST kDmaRequestMuxLPUART4Tx
#define LPUART_RX_DMA_REQUEST kDmaRequestMuxLPUART4Rx
#define EXAMPLE_LPUART_DMAMUX_BASEADDR DMAMUX
#define EXAMPLE_LPUART_DMA_BASEADDR DMA0
#define ESP8266_DEFAULT_UART_BAUDRATE  115200
#define ESP8266_DEFAULT_UART_BAUDRATE_STR  "115200"
#define ESP8266_OPERATION_UART_BAUDRATE  4000000
#define ESP8266_OPERATION_UART_BAUDRATE_STR  "4000000"
//#define ESP8266_OPERATION_UART_BAUDRATE  115200
//#define ESP8266_OPERATION_UART_BAUDRATE_STR  "115200"
#define RX_RING_BUFFER_SIZE 64
/*******************************************************************************
 * Variables
 ******************************************************************************/

lpuart_edma_handle_t g_lpuartEdmaHandle;
lpuart_handle_t g_lpuartHandle;
edma_handle_t g_lpuartTxEdmaHandle;
edma_handle_t g_lpuartRxEdmaHandle;
lpuart_config_t g_lpuartConfig;
uint8_t g_rxRingBuffer[RX_RING_BUFFER_SIZE] = {0}; /* RX ring buffer. */
size_t recv_done = 0;
int send_done = 0;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPUART AT callback  */
static void LPUART_AT_Callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *at_Data)
{
    if (kStatus_LPUART_RxIdle == status)
    {
         recv_done = handle->txDataSizeAll;
    }
    if (kStatus_LPUART_TxIdle == status)
    {
    	send_done = 1;
    }


//    struct esp_Data_t esp_dat = (struct esp_Data_t *)at_Data;
//		if (esp_dat->user_handle) {
//			esp_dat->user_handle(status, esp_dat->userData);
//		}
}

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief LPUART_esp8266_Callback function
 *  Callback the user_handle function after sending/receiving communication data
 */
static void LPUART_esp8266_Callback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *esp_Data)
{
    struct esp_Data_t *esp_dat = (struct esp_Data_t *)esp_Data;

	 if (esp_dat->user_handle) {
			esp_dat->user_handle(status, esp_dat->userData);
		}
}

/*!
 * @brief esp8266_uart_init function
 */
int esp8266_uart_init(struct esp_Data_t *esp_Data)
{
    edma_config_t config;
    /* Initialize the LPUART. */
    /*
     * lpuartConfig.baudRate_Bps = 115200U;
     * lpuartConfig.parityMode = kLPUART_ParityDisabled;
     * lpuartConfig.stopBitCount = kLPUART_OneStopBit;
     * lpuartConfig.txFifoWatermark = 0;
     * lpuartConfig.rxFifoWatermark = 0;
     * g_lpuartConfig.enableTx = false;
     * g_lpuartConfig.enableRx = false;
     */
    LPUART_GetDefaultConfig(&g_lpuartConfig);
    g_lpuartConfig.baudRate_Bps = ESP8266_DEFAULT_UART_BAUDRATE;
    g_lpuartConfig.enableTx = true;
    g_lpuartConfig.enableRx = true;

                                
    LPUART_Init(AT_LPUART, &g_lpuartConfig, AT_LPUART_CLK_FREQ);

//#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
//    /* Init DMAMUX */
//    DMAMUX_Init(EXAMPLE_LPUART_DMAMUX_BASEADDR);
//    /* Set channel for LPUART */
//    DMAMUX_SetSource(EXAMPLE_LPUART_DMAMUX_BASEADDR, LPUART_TX_DMA_CHANNEL, LPUART_TX_DMA_REQUEST);
//    DMAMUX_SetSource(EXAMPLE_LPUART_DMAMUX_BASEADDR, LPUART_RX_DMA_CHANNEL, LPUART_RX_DMA_REQUEST);
//    DMAMUX_EnableChannel(EXAMPLE_LPUART_DMAMUX_BASEADDR, LPUART_TX_DMA_CHANNEL);
//    DMAMUX_EnableChannel(EXAMPLE_LPUART_DMAMUX_BASEADDR, LPUART_RX_DMA_CHANNEL);
//#endif
    /* Init the EDMA module */
//    EDMA_GetDefaultConfig(&config);
//    EDMA_Init(EXAMPLE_LPUART_DMA_BASEADDR, &config);
//    EDMA_CreateHandle(&g_lpuartTxEdmaHandle, EXAMPLE_LPUART_DMA_BASEADDR, LPUART_TX_DMA_CHANNEL);
//    EDMA_CreateHandle(&g_lpuartRxEdmaHandle, EXAMPLE_LPUART_DMA_BASEADDR, LPUART_RX_DMA_CHANNEL);

//    /* Create LPUART DMA handle. */
//    LPUART_TransferCreateHandleEDMA(AT_LPUART, &g_lpuartEdmaHandle, LPUART_esp8266_Callback, esp_Data, &g_lpuartTxEdmaHandle,
//                                    &g_lpuartRxEdmaHandle);
   // LPUART_TransferCreateHandle(AT_LPUART, &g_lpuartHandle, LPUART_AT_Callback, &at_Data);
    LPUART_TransferCreateHandle(AT_LPUART, &g_lpuartHandle, LPUART_AT_Callback, NULL);
    LPUART_TransferStartRingBuffer(AT_LPUART, &g_lpuartHandle, g_rxRingBuffer, RX_RING_BUFFER_SIZE);
   

		return 0;
}

void esp8266_reset(void) // total 800ms
{
	 GPIO_PinWrite(GPIO2, 24, 0);   			// Module nCS==ESP8266 GPIO15 as well, Low during reset in order for a normal reset 
	 delay_ms(1); 	    		// delay 1ms, adequate for nCS stable
	
	 GPIO_PinWrite(GPIO1, 13, 0);					// Pull low the nReset Pin to bring the module into reset state
	 delay_ms(5);      		// delay 1ms, adequate for nRESET stable.
																					// give more time especially for some board not good enough
	
	 GPIO_PinWrite(GPIO1, 13, 1);					// Pull high again the nReset Pin to bring the module exiting reset state
	 delay_ms(300); 	  		// at least 18ms required for reset-out-boot sampling boottrap pin
																					// Here, we use 300ms for adequate abundance, since some board GPIO,
																					// needs more time for stable(especially for nRESET)
																					// You may shorten the time or give more time here according your board v.s. effiency
 	 GPIO_PinWrite(GPIO2, 24, 1);
 //M8266WIFI_Module_delay_ms(1); 	    		// delay 1ms, adequate for nCS stable
	
   delay_ms(800-300-5-2); // Delay more than around 500ms for M8266WIFI module bootup and initialization 
																					 // including bootup information print¡£ 
}

/*!
 * @brief  connect to AP
 */
int esp8266_wifi_setup(char *ssid, char *password)
{
    char ssid_b[32];
	/* Send AT test cmd */
	//  SysTick_Init();
	  while (AT_CMD_AT_TSET());    // AT Test 
		
		
		AT_CMD_RESET();     // reset ESP8266 
		
		if (AT_CMD_AT_TSET()){    // AT Test with echo closed
			return 1;
		}
	
	
		if (!AT_CMD_GET_WIFI_STATE(ssid_b)){  	
			if (!strcmp(ssid, ssid_b)){
				/* Has connected to AP */
				return 0;
			}
	  }
		AT_CMD_DISCONNECT_AP();
		AT_CMD_SET_WIFI_MODE("1");   // work on station mode
		while (1) {
			if (!AT_CMD_CONNECT_AP(ssid, password)){     
				return 0;
			}
		}
}

/*!
 * @brief  create a tcp connection 
 */
int esp8266_create_tcp_client(char *ip, char *port)
{
	char status[3][32];
	if (!AT_CMD_GET_CONNECT_STATE(status)){  	
		if (!strcmp(status[0], "3")){
			/* Has connected to a tcp  transmission*/
			if (!strcmp(ip, status[1]))
				if (!strcmp(port, status[2]))
					return 0;
			AT_CMD_DISCONNECT_TCP();
		}
		else if (strcmp(status[0], "2")){
				return 1;     //  doesn't connect to AP
		} 
	}
	AT_CMD_SET_CIPMUX();      // set singel-connection  mode
	while (1) {
		if (!AT_CMD_CONNECT_TCP(ip, port)){    
			break;
	  }
	}
			if (AT_CMD_UP_BAUDRATE(ESP8266_OPERATION_UART_BAUDRATE_STR)){  // Update baudRate  
			return 1;
		}
		
    LPUART_Deinit(AT_LPUART);
		g_lpuartConfig.baudRate_Bps = ESP8266_OPERATION_UART_BAUDRATE;
		LPUART_Init(AT_LPUART, &g_lpuartConfig, AT_LPUART_CLK_FREQ);
		
		if (AT_CMD_AT_TSET()){    // AT Test with echo closed
			return 1;
		}
	AT_CMD_TRANSIMITTAL_MODE();   // After creating the tcp connection, esp8266 should switch transmittal mode.
//	LPUART_TransferStopRingBuffer(AT_LPUART,  &g_lpuartHandle);
	return 0;
}
/*!
 * @brief  disconnect the tcp connection 
 */
void esp8266_disconnect_client(void)
{
	size_t count;
	
//	LPUART_TransferAbortSendEDMA(AT_LPUART, &g_lpuartEdmaHandle);
//	while (LPUART_TransferGetSendCountEDMA(AT_LPUART, &g_lpuartEdmaHandle, &count) != kStatus_NoTransferInProgress);
//	LPUART_TransferAbortReceiveEDMA(AT_LPUART, &g_lpuartEdmaHandle);
//	LPUART_TransferStartRingBuffer(AT_LPUART, &g_lpuartHandle, g_rxRingBuffer, RX_RING_BUFFER_SIZE);
	AT_CMD_AT_MODE();  //   Switch to AT mode from transmittal mode

	if (!AT_CMD_UP_BAUDRATE(ESP8266_DEFAULT_UART_BAUDRATE_STR )){  // Update baudRate  
			    LPUART_Deinit(AT_LPUART);
					g_lpuartConfig.baudRate_Bps = ESP8266_DEFAULT_UART_BAUDRATE;
					LPUART_Init(AT_LPUART, &g_lpuartConfig, AT_LPUART_CLK_FREQ);
		}
	
			AT_CMD_DISCONNECT_TCP();
}

/*!
 * @brief  disconnect the tcp connection 
 */
void esp8266_disconnect_wifi(void)
{
	AT_CMD_RESET();
}

/*!
 * @brief  send date 
 */
void esp8266_send(void *buf, size_t size)
{
    lpuart_transfer_t xfer;  
	  xfer.data = buf;
    xfer.dataSize = size;
//    LPUART_SendEDMA(AT_LPUART, &g_lpuartEdmaHandle, &xfer);
}

/*!
 * @brief  receive date 
 */
void esp8266_recv(void *buf, size_t size)
{
    lpuart_transfer_t xfer;  
	  xfer.data = buf;
    xfer.dataSize = size;
//    LPUART_ReceiveEDMA(AT_LPUART, &g_lpuartEdmaHandle, &xfer);
}

/*!
 * @brief  receive abort 
 */
void esp8266_abort_recv()
{
//    LPUART_TransferAbortSendEDMA (AT_LPUART, &g_lpuartEdmaHandle);
}

/*!
 * @brief  send AT date 
 */
void esp8266_at_send(void *buf, size_t size)
{
    lpuart_transfer_t xfer;  
	  xfer.data = buf;
    xfer.dataSize = size;
    send_done = 0;
    LPUART_TransferSendNonBlocking(AT_LPUART, &g_lpuartHandle, &xfer);
}

void wait_send_done()
{
	while(!send_done);
}

/*!
 * @brief  receive AT date 
 */
size_t esp8266_at_recv(void *buf, size_t size)
{
    size_t has_recv;
	  lpuart_transfer_t xfer;  
	  xfer.data = buf;
    xfer.dataSize = size;
    LPUART_TransferReceiveNonBlocking(AT_LPUART, &g_lpuartHandle, &xfer, &has_recv); 
	  return has_recv;
}

/*!
 * @brief  receive abort AT date 
 */
size_t esp8266_at_abort_recv()
{ 
	uint32_t count;	 
	if (LPUART_TransferGetReceiveCount(AT_LPUART, &g_lpuartHandle, &count) != kStatus_Success)
	{
		return recv_done;
	}
	LPUART_TransferAbortReceive(AT_LPUART, &g_lpuartHandle); 
  return count;	
}
void esp8266_at_stop_ring()
{ 
	LPUART_TransferStopRingBuffer(AT_LPUART,  &g_lpuartHandle);
}

void esp8266_at_start_ring()
{ 
//	LPUART_TransferStartRingBuffer(AT_LPUART, &g_lpuartHandle, g_rxRingBuffer, RX_RING_BUFFER_SIZE);
} 

