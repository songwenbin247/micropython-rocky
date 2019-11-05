
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "py/binary.h"
#include "py/runtime.h"
#include "py/mphal.h" 
#include "py/objstr.h"
#include "py/obj.h"
//#include "qstrdefs.generated.h"
#include "portmodules.h"
#include "esp8266.h"
#include "ff.h"
#include "ff_wrapper.h"
#include "fsl_cache.h"
#include "imlib.h"
typedef void (*TX_next_t)(void );
typedef enum {
	REQUEST_IDLE = 0,
	REQUEST_PICTURE = 1,
	REQUEST_COORDINATE = 2,
	SENDING = 3,
} action_t;


/*
 * request frame:  
 *    HEADER: 0x54 0x45  
 *    CMD:    0x33   get a picture.
 *            0x65   get a  coordinate 
*    END:     0x48
 */     

typedef struct {
    char *data;
	  size_t size;
	  FIL fp;	
    int isDone;	
}send_data_t;  

typedef struct 
{
	char  ssid[32];
  char  password[16];
  bool  isConnected;
  char  ip[18];
	char  port[8];
  bool  isTcpConnected;	
	struct esp_Data_t esp_Data;
	TX_next_t tx_next;
	action_t action_next;
	char receive; 
	send_data_t send_data;
	size_t cmd_len;
} station_obj_t; 

station_obj_t *self;


void finshed_callback (status_t status, void *userData)
{
	char rec = self->receive;
	if (status == RxIdle) {
		if (self->action_next == REQUEST_IDLE ){
			switch (self->cmd_len){
				case 0:
					if (rec== 'g'){
							self->cmd_len= 1;
					}
					break;
				case 1:
					if (rec== 'e'){
							self->cmd_len= 2;
					}else {
							self->cmd_len= 0;
					}
					break;
				case 2:
					if (rec== 't'){
							self->cmd_len= 3;
					}else {
							self->cmd_len= 0;
					}					
					break;
				case 3:
					if (rec== '_'){
							self->cmd_len= 4;
					}else {
							self->cmd_len= 0;
					}					
					break;
				case 4:
					if (rec== 'p'){
							self->action_next = REQUEST_PICTURE;;
					}else if( rec== 'c') {
							self->action_next = REQUEST_COORDINATE;
					}
					self->cmd_len= 0;
					break;
				  
			}
		}	 
		esp8266_recv(&self->receive, 1);
	}
	else if (status == TxIdle) {
		   if (self->tx_next)
					self->tx_next();
			
	}
}

	mp_obj_t station_wifi(size_t n_args, const mp_obj_t *args)
	{
	
	  self = m_new_obj(station_obj_t);
		if(n_args == 2){
			strcpy(self->ssid, mp_obj_str_get_str(args[0]));
			strcpy(self->password, mp_obj_str_get_str(args[1]));
		}
	  
		if(n_args == 4){
			strcpy(self->ip, mp_obj_str_get_str(args[2]));
			strcpy(self->port, mp_obj_str_get_str(args[3]));
		} 
	  self->isConnected = false;
		self->isTcpConnected = false;
		self->esp_Data.user_handle = finshed_callback;
		self->esp_Data.userData = self ;
		self->cmd_len = 0;
		esp8266_uart_init(&self->esp_Data);
		self->tx_next = NULL;
		self->action_next = REQUEST_IDLE;
		return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(station_wifi_obj,0, 4, station_wifi);

//STATIC void station_print( const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind )
//{
//		station_obj_t *self = MP_OBJ_TO_PTR(self);
//	//  mp_printf(print, "SSID: %s PASSWORD: %s STATUS: %s\n", self->ssid, self->password, self->isConnected ? "Connected": "No connected" );
//}

STATIC mp_obj_t connect_ap(size_t n_args, const mp_obj_t *args )
{
	char  ssid[16];
  char  password[16];
	if (n_args == 2) {
			strcpy(ssid, mp_obj_str_get_str(args[0]));
			strcpy(password, mp_obj_str_get_str(args[1]));
		  if (!esp8266_wifi_setup(ssid, password)) {
			    strcpy(self->ssid, ssid);
					strcpy(self->password, password);
					self->isConnected = true;
			}
	} else {
			if (!esp8266_wifi_setup(self->ssid, self->password)) {
					self->isConnected = true;
			}
	}
	self->isConnected = false;    
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(connect_ap_obj, 0, 2, connect_ap);

STATIC mp_obj_t reset()
{
	esp8266_reset();
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(reset_obj, reset);


STATIC mp_obj_t create_tcp_client(size_t n_args, const mp_obj_t *args )
{
	char  ip[16];
  char  port[8];
	if (n_args == 2) {
			strcpy(ip, mp_obj_str_get_str(args[0]));
			strcpy(port, mp_obj_str_get_str(args[1]));
		  if (!esp8266_create_tcp_client(ip, port)) {
			    strcpy(self->ip, ip);
					strcpy(self->port, port);
					self->isTcpConnected = true;
//					esp8266_recv(&self->receive, 1);
			}
	} else {
			if (!esp8266_create_tcp_client(self->ip, self->port)) {
					self->isTcpConnected = true;
//					esp8266_recv(&self->receive, 1);
			}
	}
	self->isTcpConnected = false;    
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(create_tcp_client_obj, 0, 2, create_tcp_client);

STATIC mp_obj_t del_tcp_client( )
{
    esp8266_disconnect_client();
		esp8266_abort_recv();
		self->isTcpConnected = false;
		self->tx_next = NULL;
		self->action_next = REQUEST_IDLE;
	  return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(del_tcp_client_obj, del_tcp_client);

void picture_send()
{
		self->send_data.isDone =  1;
}

//STATIC mp_obj_t send_a_picture( mp_obj_t path_obj )
//{
//	const char *path;	
//	size_t len;
//	if ((self->send_data.data = malloc(2052)) == NULL){
//			if ((self->send_data.data = malloc(1028)) == NULL){
//					return mp_const_none;
//			}
//			else {
//				self->send_data.size = 1024;
//			}
//		} else {
//			self->send_data.size = 2048;
//		}    
//    path = mp_obj_str_get_str(path_obj);
//		file_read_open(&(self->send_data.fp), path);
//		self->send_data.isDone =  1;
//		self->tx_next	= picture_send;
//		while(1){	
//			while ( self->send_data.isDone != 1);
//			if (f_read(&(self->send_data.fp), (char *)((uint32_t)(self->send_data.data + 3) & ~0x3), self->send_data.size, &len) == FR_OK) {
//					DCACHE_CleanByRange(((uint32_t)(self->send_data.data + 3) & ~0x3),  self->send_data.size);
//					self->send_data.isDone =  0;					
//					esp8266_send((char *)((uint32_t)(self->send_data.data + 3) & ~0x3), len);
//			}else {
//					break;
//			}
//			if (f_eof(&(self->send_data.fp)))
//				break;
//		}			
//		self->tx_next	= NULL;
//		f_close(&(self->send_data.fp));		 
//		free(self->send_data.data);
//		self->action_next = REQUEST_IDLE;		
//	  return mp_const_none;
//}
//MP_DEFINE_CONST_FUN_OBJ_1(send_a_picture_obj, send_a_picture);

STATIC mp_obj_t send_a_picture( mp_obj_t img_obj )
{
		image_t *arg_img = py_image_cobj(img_obj);
		size_t size = image_size(arg_img);
		size_t len = 0, offsetof = 0;
		if ((self->send_data.data = malloc(2052)) == NULL){
				if ((self->send_data.data = malloc(1028)) == NULL){
						return mp_const_none;
				}
				else {
					self->send_data.size = 1024;
				}
		} else {
				self->send_data.size = 2048;
		}    
    
		self->send_data.isDone =  1;
		self->tx_next	= picture_send;
		while(size){	
			while ( self->send_data.isDone != 1);
			len = size < self->send_data.size ? size : self->send_data.size;
			memcpy(((uint32_t)(self->send_data.data + 3) & ~0x3), arg_img->data + offsetof, len);
			DCACHE_CleanByRange(((uint32_t)(self->send_data.data + 3) & ~0x3),  self->send_data.size);
			self->send_data.isDone =  0;					
			esp8266_send((char *)((uint32_t)(self->send_data.data + 3) & ~0x3), len);
			size -= len;
			offsetof += len;
		}			
		self->tx_next	= NULL;	 
		free(self->send_data.data);
		self->action_next = REQUEST_IDLE;		
	  return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(send_a_picture_obj, send_a_picture);

void string_send()
{
	self->tx_next	= NULL;
	self->action_next = REQUEST_IDLE;	
}
char buf[256];
STATIC mp_obj_t send_a_string(mp_obj_t str_obj )
{
    const char *string;	
    size_t len;
	   
	string = mp_obj_str_get_str(str_obj); 
    len = strlen(string);			
    strncpy(buf,string, 256);			
	wait_send_done();
	esp8266_at_send(buf, len); 		
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(send_a_string_obj, send_a_string);

STATIC mp_obj_t get_action()
{
    mp_obj_t obj;
	  if (self->action_next == REQUEST_COORDINATE)
			obj = mp_obj_new_str_of_type(&mp_type_str, "Coordinate", 10);
		else if (self->action_next == REQUEST_PICTURE)
			obj = mp_obj_new_str_of_type(&mp_type_str,"Picture", 7);
		else 
			obj = mp_obj_new_str_of_type(&mp_type_str, "Idle", 4);
	  return obj;
}
MP_DEFINE_CONST_FUN_OBJ_0(get_action_obj, get_action);

//STATIC const mp_rom_map_elem_t station_locals_dict_table[] = {
//    { MP_ROM_QSTR(MP_QSTR_connect_ap), MP_ROM_PTR(&connect_ap_obj) },
//    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&reset_obj) },
//    { MP_ROM_QSTR(MP_QSTR_disconnect_ap), MP_ROM_PTR(&reset_obj) },
//    { MP_ROM_QSTR(MP_QSTR_create_tcp_client), MP_ROM_PTR(&create_tcp_client_obj) },
//    { MP_ROM_QSTR(MP_QSTR_del_tcp_client), MP_ROM_PTR(&del_tcp_client_obj) },
//    { MP_ROM_QSTR(MP_QSTR_send_a_picture ), MP_ROM_PTR(&send_a_picture_obj) },
//    { MP_ROM_QSTR(MP_QSTR_send_a_string), MP_ROM_PTR(&send_a_string_obj) },
//    { MP_ROM_QSTR(MP_QSTR_get_a_action), MP_ROM_PTR(&get_action_obj) },
//};

//STATIC MP_DEFINE_CONST_DICT(station_locals_dict, station_locals_dict_table);
//const mp_obj_type_t station_wifi_obj = {
//	 {&mp_type_type},
//	.name = MP_QSTR_esp8266_station_mode,
//	.print = station_print,
//	.make_new = station_make_new,
//	.locals_dict = (mp_obj_dict_t *)&station_locals_dict,
//};
//	
STATIC const mp_rom_map_elem_t  esp8266_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_esp8266_wifi)},
	{MP_ROM_QSTR(MP_QSTR_esp8266_station), (mp_obj_t)&station_wifi_obj},
	{ MP_ROM_QSTR(MP_QSTR_connect_ap), (mp_obj_t)&connect_ap_obj },
  { MP_ROM_QSTR(MP_QSTR_reset), (mp_obj_t)&reset_obj},
  { MP_ROM_QSTR(MP_QSTR_disconnect_ap), (mp_obj_t)&reset_obj },
  { MP_ROM_QSTR(MP_QSTR_create_tcp_client), (mp_obj_t)&create_tcp_client_obj },
  { MP_ROM_QSTR(MP_QSTR_del_tcp_client), (mp_obj_t)&del_tcp_client_obj },
  { MP_ROM_QSTR(MP_QSTR_send_a_picture ), (mp_obj_t)&send_a_picture_obj },
  { MP_ROM_QSTR(MP_QSTR_send_a_string), (mp_obj_t)&send_a_string_obj },
  { MP_ROM_QSTR(MP_QSTR_get_a_action), (mp_obj_t)&get_action_obj },
};

STATIC MP_DEFINE_CONST_DICT (
	esp8266_wifi_globales, esp8266_globals_table
);

const mp_obj_module_t mp_module_esp8266_wifi = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&esp8266_wifi_globales,
};
