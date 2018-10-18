// Contact: <gegelcopy@ukr.net>
// Author: Sergey Gayevsky
//
// THIS IS A FREE SOFTWARE
//
// This software is released under GNU LGPL:
//
// * LGPL 3.0 <http://www.gnu.org/licenses/lgpl.html>
//
// You're free to copy, distribute and make commercial use
// of this software under the following conditions:
//
// * You have to cite the author (and copyright owner): Sergey Gayevsky
// * You have to provide a link to the author's Email: <gegelcopy@ukr.net>


#include <stdint.h>
//timing
extern uint32_t tnow;
extern uint32_t tout;

//data processing
extern uint32_t rxbufd[];  //processing buffer alligned to int
extern uint8_t* rxbuf; //this buffer for uint8_t
extern int16_t rxlen; //data length in rxbuf will be processed

//sms data processing buffer
extern uint8_t sms_number[];
extern uint8_t smsbuf[];
extern uint8_t smslen;
extern uint8_t smsflag;
extern uint16_t smsrep;

//data output
extern uint8_t txbuf[]; //output buffer (data for allication or for gsm module uart)
extern int16_t txlen; //data length

//dtmf data
extern uint8_t dtmfbuf[];
extern uint8_t dtmflen;

//states
extern uint32_t state; //32 bit flags
extern uint8_t insend; // flag of in gprs send procedure (wait promt > for output binary data)
extern int16_t rxdata; //length of received gprs binary data will be processed
extern int16_t errcode; //last error code

//state values
extern uint8_t reg_value; //current GSM registration
extern uint8_t gatt_value;  //current GPRS avaliable
extern uint8_t call_number[32]; //number of incoming call
extern uint8_t ip[16];  //gprs local ip

//internal settings
extern int8_t str[32]; //apn, dest for connect
extern uint16_t internal_port; //dest port for connect, send
extern uint8_t internal_ch;   //socket for connect, send, close
extern uint8_t internal_tcp;  //protocol for connect
extern uint8_t internal_len;  //dta len for send

//pointers to callback functions for curently executed AT command
extern void (*cb_ok)(void);
extern void (*cb_er)(void);
extern void (*cb_to)(void);
