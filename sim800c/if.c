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



#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "def.h"
#include "urc.h"
#include "atc.h"
#include "ini.h"
#include "if.h"
//-----------------------------------------
//data receiving from UART
uint8_t ccbuf[256];  //circular rx buffer
volatile uint8_t inptr=0; //pointer to input data were received
uint8_t outptr=0;   //pointer to utput data will be processed

//sms data processing buffer
uint8_t sms_number[32]={0};
uint8_t smsbuf[MAXSMSLEN];
uint8_t smsflag=0;
uint8_t smslen=0;
uint16_t smsrep=0;

//gprs data processing buffer
uint32_t rxbufd[MAXDATALEN/sizeof(int)];  //processing buffer alligned to int
uint8_t* rxbuf = (uint8_t*) rxbufd; //this buffer for uint8_t
int16_t rxlen=0; //data length in rxbuf will be processed

//data output
uint8_t txbuf[MAXDATALEN]; //output buffer (data for allication or for gsm module uart)
int16_t txlen=0; //data length

//dtmf data
uint8_t dtmfbuf[32];
uint8_t dtmflen=0;

//timing
uint32_t tnow=0; //current timestamp in seconds
uint32_t tout=0; //timestamp for timeout of processed command

//states
uint32_t state=0; //32 bit flags
uint8_t insend=0; // flag of in gprs send procedure (wait promt > for output binary data)
int16_t rxdata=0; //length of received gprs binary data will be processed
int16_t errcode=0; //last error code

//state values
uint8_t reg_value=0; //current GSM registration
uint8_t gatt_value=0;  //current GPRS avaliable
uint8_t call_number[32]={0}; //number of incoming/outgoing call
uint8_t ip[16]={0xFF};  //gprs local ip

//internal settings
int8_t str[32]={0}; //apn, dest for connect
uint16_t internal_port=0; //dest port for connect, send
uint8_t internal_ch=0;   //socket for connect, send, close
uint8_t internal_tcp=0;  //protocol for connect
uint8_t internal_len=0;  //dta len for send

//pointers to callback functions for curently executed AT command
void (*cb_ok)(void)=0;
void (*cb_er)(void)=0;
void (*cb_to)(void)=creq_go;

//===============================================================
//task
//===============================================================
uint32_t poll(uint32_t tmt) //OK task, returns state
{
 int16_t ptr = (int16_t)inptr; //shadow volatile input data pointer
 int16_t len; //new data avaliable
 int16_t i;
 uint8_t c;


 //-------------------------------------------
 //output TX data with return
 if(txlen)
 {
  state |= STATE_TX;  //set TX data ready flag
  return state;  //return for reading data by application
 }
 //-------------------------------------------


 //-------------------------------------------
 //process new RX data
 if(ptr!=outptr) //if some new data
 {
  len = ptr-(int16_t)outptr; //data length
  if(len<0) len+=sizeof(ccbuf); //ring
  for(i=0;i<len;i++) //process byte-by-byte
  {
   c=ccbuf[outptr++];  //get next received byte
   rxbuf[rxlen++]=c;  //add to total

   if(rxdata) //check for gprs binary data receved
   {
    rxdata--;
    if(!rxdata) //check for end of data
    {
     state |= STATE_DATA;  //set data ready flag
     return state;  //return for reading data by application
    }
   } //end of gprs data processing
   else if(c==0x0A) //URC received: check for <lf>
   {
    rxprocess(rxlen); //process AT string
    rxlen=0; //clear pointer
    if(smsflag && smslen) //check for SMS data ready
    {
     state |= STATE_SMS;  //set TX data ready flag
     smsflag=0;  //clear sms flag
     return state;  //return for reading data by application
    }
   } //end of URC processing
   else if(insend && (c=='>')) //into send procedure: wait promt > for data will be sended
   {
    data_send(); //output data will be sended over gprs
    insend=0; //clear insend flag
    state |= STATE_TX;  //set TX data ready flag
    return state; //return for sending data to uart
   }
  } //end of byte-by-byte processing
 }   //end of new RX data processing
 //-------------------------------------------


 //-------------------------------------------
 //check for tout
 tnow=tmt; //set new time
 if(tnow>tout)
 {
  if(cb_to)
  {
   cb_to(); //call tout function if not zero
  }
 }
 //-------------------------------------------


 return state;
}


//===============================================================
//hw AT module interface
//===============================================================
//add data received over ouart
void rx(int8_t* data, int16_t len)  //OK add data received over uart
{
 int16_t i;
 for(i=0;i<len;i++) ccbuf[inptr++]=data[i];
}


//===============================================================
//get data functions
//===============================================================
//get data for transmitting over uart
int16_t tx(int8_t* data)  //OK get data for transmit over uart
{
 int16_t len=txlen;
 if(data) memcpy(data, txbuf, len);
 txlen=0;
 state &= (~STATE_TX); //clear TX flag
 return len;
}
//---------------------------------------------------------------
//get data received over gprs
int16_t readdata(uint8_t* data, uint8_t* chn) //OK read received data, returns len
{
 int16_t len=rxlen;
 if(chn) chn[0]=internal_ch;
 if(data) memcpy(data, rxbuf, len);
 rxlen=0;
 state &= (~STATE_DATA); //clear data flag
 return len;
}

//---------------------------------------------------------------
//get data received over sms
int16_t readsms(uint8_t* data, uint8_t* phone) //OK read received data, returns len
{
 int16_t len=smslen-1; //data length
 if(len<0) return 0;
 if(data) //check for output buffer specified
 {
  memcpy(data, rxbuf, len); //output sms data
  data[len]=0; //terminate string
 }
 if(phone) strcpy(phone, sms_number); //check for output buffer specified and output source phone

 state &= (~STATE_SMS); //clear sms flag
 smslen=0; //clear sms 
 return len; //return data len
}



//===============================================================
//user commands
//===============================================================
//activate/deactivate gprs
void gprs(int8_t* apn) //OK
{
 if(apn) //activate gprs
 {
  strncpy(str, apn, sizeof(str)); //save apn
  cgatt_go(); //start gprs activation sequence
 }
 else cipshut_go();//deactivate gprs

}
//-------------------------------------------------------------

void listen(uint16_t port) //OK run/stop tcp listener
{
 if(port) //start server
 {
  internal_port=port; //save local port
  cipserver1_go();  //start server
 }
 else cipserver0_go(); //close server
}

//-------------------------------------------------------------
//conect as a gprs client  OK
void connect(uint8_t ch, uint8_t tcp, int8_t* dest, uint16_t port) //connect to remote server
{
 if(ch>4) return; //check channel must be 0-4
 internal_ch=ch;
 internal_tcp=tcp;

 if(dest && dest[0] && port)
 {
  strcpy(str, dest);
  internal_port=port;
  cipstart_go(); //start client
 }
 else cipclose_go();  //else close socket: stop client


}

void close(uint8_t ch)
{
 if(ch>4) return; //check channel must be 0-4
 internal_ch=ch;
 cipclose_go();
}
//-------------------------------------------------------------
//send data over gprs
void send(uint8_t ch, int8_t* data, uint16_t len)
{
 if((ch>4)||(!len)||(len>(MAXDATALEN-32))) return;
 memcpy(txbuf+32, data, len);
 internal_len=len;
 internal_ch=ch;
 cipsend_go();
}

//send data over sms
//-------------------------------------------------------------
void sendsms(uint8_t* phone, int8_t* data)
{
 int16_t len=strlen(data);
 if((!len)||(len>MAXSMSLEN)) return; //check is len range
 strcpy(sms_number, phone);
 memcpy(txbuf+32, data, len); //copy sms data to txbuf
 txbuf[len+32]=0x1A; //add ctrl-z sms string terminator
 internal_len=len+1; //len of data + terminator
 smssend_go(); //send AT command
}

//delete all sms
//-------------------------------------------------------------
void delallsms(void)
{
 cmgd_go();
}

//-------------------------------------------------------------
//get sms delivery code, clear delivering flag 
int16_t smsd(void)
{
 int16_t rep = smsrep;
 smsrep=0;
 state &= (~STATE_SMSD);
 return rep;
}

//-------------------------------------------------------------
//voice call managment:
//if number is specified: start voice call
//if number is empty: answer to incoming call
//if number is NULL: stop call
void call(int8_t* num)
{
 if(!num) ath_go();
 else if(!num[0]) ata_go();
 else
 {
  strncpy(call_number, num, 32);
  atd_go();
 }
}

void answer(void)
{
 ata_go();
}

void reject(void)
{
 ath_go();
}

void reinit(void)
{
 creini_go();
}

//get number of incoming call to num (as a string)
void callnum(uint8_t* num)
{
 if(num)
 {
  if(state & STATE_CNUM) strcpy(num, call_number);
  else num[0]=0;
 }
 state &= (~STATE_CNUM);
}

//read received dtmf tones to data, returns number of tones readed
int16_t readdtmf(uint8_t* data)
{
 int16_t len = 0;

 if(state&STATE_DTMF)
 {
  len=dtmflen;
  if(data)
  {
   memcpy(data, dtmfbuf, len);
   data[len]=0;
  }
 }
 dtmflen=0;
 state &= (~STATE_DTMF);
 return len;
}

void senddtmf(uint8_t* data, int16_t len)
{
 if(len>31) len=31;
 if(!len) return;
 memcpy(dtmfbuf, data, len);
 dtmflen=len;
 dtmf_go();
}

int16_t lasterr(void)
{
 state &= (~STATE_ERROR);
 return errcode;
}







