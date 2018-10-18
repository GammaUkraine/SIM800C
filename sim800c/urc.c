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
#include "data.h"
#include "atc.h"
#include "urc.h"



//================================================================
//Parser of AT answers and URC codes,
//processing
//================================================================


//URC bytes 0,1,2,3
typedef union
{
 const uint8_t b[16][4];
 const uint32_t d[];
}URC_data0;

URC_data0 URC0={
'O','K',0xD,0xA, //OK
'E','R','R','O', //ERROR
'+','C','R','E', //+CREG:
'+','C','M','T', //+CMT: (incoming SMS)
'+','R','E','C', //+RECEIVE: (incoming gprs)
'R','I','N','G', //RING (incoming call)
'+','C','L','I', //+CLIP: (incoming call number
'N','O',' ','C', //NO CARRIER (incoming call terminated by remote
'B','U','S','Y', //BUSY  (outgoing call rejected)
'C','a','l','l', //Call Ready (on start)
'S','M','S',' ', //SMS Ready (on start)
'S','E','R','V', //SERVER
'+','C','G','A', //+CGATT
'S','H','U','T', //SHUT OK
'+','C','M','G', //SMS send
'N','O',' ','A'  //NO ANSWER
};


#define U0_OK 0
#define U0_ER 1
#define U0_CREG 2
#define U0_CMT 3

#define U0_RCVD 4
#define U0_RING 5
#define U0_CLIP 6
#define U0_NOCR 7

#define U0_BUSY 8
#define U0_CR 9
#define U0_SR 10
#define U0_SRV 11

#define U0_GATT 12
#define U0_SHUT 13
#define U0_SMS 14
#define U0_NOANS 15


//URC bytes 4,5,6,7
typedef union
{
 const uint8_t b[8][4];
 const uint32_t d[];
}URC_data1;


URC_data1 URC1={
'E','N','D',' ', //SEND  (2)
'O','N','N','E', //CONNECT (2)
'E','M','O','T', //REMOTE accept incoming tcp (2)
'L','O','S','E', //CLOSE socket close (2)
'N','O',' ','D', //NO DIALTONE (1)
'+','D','T','M', //+DTMF: (receive dtmf tone)
' ',' ',' ',' ',
' ',' ',' ',' '
};



#define U1_SEND 0
#define U1_CONNECT 1
#define U1_ACCEPT 2
#define U1_CLOSE 3
#define U0_NODT 4
#define U0_DTMF 5

//===============================================================
//Parser: process module answer in rxbuf
//===============================================================
void rxprocess(int16_t len)
{
 uint32_t d0;
 uint32_t d1;
 //URC PARSER

 if(smsflag && (!smslen)) //check for sms data expected
 {
  if((len>1)&&(len<MAXSMSLEN)) smslen=len-1; //check data length
  else smsflag=0; //or clear flag for incorrect length
 }

 //now we detect URC type by first 4 or next 4 bytes and parse in correspond subcall
 if(len<4) return;
 d0=rxbufd[0]; //first 4 bytes of URC
 if(len>7) d1=rxbufd[1]; //next 4 bytes of URC
 else d1=0;


 if((d0==URC0.d[U0_OK])&& (cb_ok!=0)) cb_ok();
 else if((d0==URC0.d[U0_ER]) && (cb_er!=0)) cb_er();
 else if(d0==URC0.d[U0_CREG]) get_reg(); //set reg value
 else if(d0==URC0.d[U0_CMT]) get_sms(); //process incoming sms
 else if(d0==URC0.d[U0_RING]) get_call(); //process incoming call
 else if(d0==URC0.d[U0_RCVD]) get_data(); //process gprs data
 else if(d1==URC1.d[U1_ACCEPT]) get_accept(); //new incoming tcp connecting
 else if(d1==URC1.d[U1_CLOSE]) get_close(); //close existing connecting
 else if(d0==URC0.d[U0_GATT]) get_att(); //gatt result
 else if(d0==URC0.d[U0_SRV]) get_srv(); //server result
 else if(d1==URC1.d[U1_CONNECT]) get_connect(); //connect result
 else if(d1==URC1.d[U1_SEND]) get_send(); //send result
 else if(d0==URC0.d[U0_SHUT]) get_shut(); //shut result
 else if(d0==URC0.d[U0_BUSY])  state &= (~STATE_CALL);
 else if(d0==URC0.d[U0_NOCR]) state &= (~STATE_CALL);
 else if(d0==URC0.d[U0_CLIP]) get_clip(); //call number result
 else if(d0==URC0.d[U0_CR]) get_cr(); //Call Ready (on start once)
 else if(d0==URC0.d[U0_SR]) get_sr(); //SMS Ready (on start once)
 else if(d0==URC0.d[U0_SMS]) get_smssend(); //sms send report
 else if( (d0==URC0.d[U0_BUSY])|| (d0==URC0.d[U0_NOCR]) || (d0==URC0.d[U0_NOANS]) || (d0==URC1.d[U0_NODT])) get_callstop();
 else if(d0==URC1.d[U0_DTMF]) get_dtmf(); //dtmf tone detected
 else if(!ip[0]) get_ip();
}

//===============================================================
//Procedures for processing
//===============================================================

//after > after AT+CIPSEND=1,4#0D  (for gprs and sms)
void data_send(void)
{
 int16_t i;
 for(i=0;i<internal_len;i++) txbuf[i]=txbuf[i+32];  //copy data to start of tx buffer
 txlen=internal_len;  //set len for TX
 internal_len=0; //clear data length
 insend=0; //clear flag for wait promt >
}
//----------------------------------------------------------------
//answer to AT+CIFSR#0D
void get_ip(void)  //OK
{
 //#0D#0A10.253.64.48#0D#0A
 int16_t i;
 //check dot in IP adress
 if((rxbuf[3]=='.')||(rxbuf[2]=='.')||(rxbuf[1]=='.'))
 { //copy ip adress to starage, replace unprintable to terminator
  for(i=0;i<15;i++) if(rxbuf[i]>0x10) ip[i]=rxbuf[i]; else ip[i]=0;
  ip[15]=0; //terminate IP string
  cifsr_ok();
 }
}
//----------------------------------------------------------------
//URC after AT+CIPSERVER=1,4444#0D
void get_srv(void) //OK
{
 //OK#0D#0A#0D#0ASERVER OK#0D#0A
 //SERVER CLOSE
 if(rxbuf[7]=='O') cipserver1_ok();
 else cipserver1_er();
}
//----------------------------------------------------------------
//URC on incoming TCP connecting was accepted
void get_accept(void) //OK
{
 //0, REMOTE IP: 10.76.40.73
 int16_t c=rxbuf[0]-0x30;
 if((c<0)||(c>7)) return;
 state |= ((uint32_t)1)<<(16+c); //set connected
}
//----------------------------------------------------------------
//URC after AT+CIPSTART=1,"TCP","94.45.70.111","3333"#0D
void get_connect(void) //OK
{
 //1, CONNECT OK
 int16_t c=rxbuf[0]-0x30;
 if((c<0)||(c>7)) return;
 if(rxbuf[11]=='O')
 {
  state |= ((uint32_t)1)<<(16+c); //set flag "connected"
  cipstart_ok();
 }
 else
 {
  state &= (~((uint32_t)1)<<(16+c));
  cipstart_er();
 }
}
//----------------------------------------------------------------
//URC after AT+CIPCLOSE=1#0D
//URC after socket closed remotely
void get_close(void) //OK
{
 //1, CLOSE OK#0D#0A   manually
 //1, CLOSED#0D#0A  remotely
 int16_t c=rxbuf[0]-0x30;
 if((c<0)||(c>7)) return;
 state &= (~(((uint32_t)1)<<(16+c))); //clear flag "connected"
 cipclose_ok();
}
//----------------------------------------------------------------
//URC on gsm registration changed
void get_reg(void) //OK
{
 //+CREG: 1
 int16_t c;
 if(rxbuf[8]==',')  //answer to AT+CREG?
 {
  if(rxbuf[7]=='1') state |= STATE_INIT;
  c=rxbuf[9]-0x30;
 }
 else  c=rxbuf[7]-0x30;//URC code


 if((c<0)||(c>5)) return;
 reg_value=c;
 if((c==1)||(c==5)) state |= STATE_REG;
 else state &= (~STATE_REG);
}
//----------------------------------------------------------------
//URC with incoming text SMS header
void get_sms(void)
{
 //+CMT: "+380958110260","","18/09/05,10:40:44+12"#0D#0A
 int16_t i;
 uint8_t c;

 //parse source number
 sms_number[0]=0;
 for(i=0;i<31;i++)
 {
  c=rxbuf[i+7];
  sms_number[i]=c;
  if(c=='"') break;
 }
 sms_number[i]=0; //terminate source number string
 smslen=0;  //clear sms data length
 smsflag=1; //set flag next URC will be SMS data
}
//----------------------------------------------------------------

//URC on incoming call
void get_call(void)
{
 //RING
 state |= STATE_CALL;
 
}
//----------------------------------------------------------------
//URC on GPRS data received
void get_data(void) //OK
{
 //+RECEIVE,1,1:#0D#0A#00
 int16_t len=0;
 int16_t c=rxbuf[9]-0x30;
 if((c<0)||(c>7)) return;
 internal_ch=c; //channel of data was received
 //obtain next data len up to 9999 bytes
 while(1)
 {
  c=rxbuf[11];  //first digit
  if((c>0x39)||(c<0x30)) break; //must be in rabhe '0'-'9'
  c-=0x30;
  len=c; //add as low decimal
  c=rxbuf[12]; //second digit
  if((c>0x39)||(c<0x30)) break; //must be in rabhe '0'-'9'
  c-=0x30;
  len=len*10+c; //add as low decimal
  c=rxbuf[13];  //third digit
  if((c>0x39)||(c<0x30)) break; //must be in rabhe '0'-'9'
  c-=0x30;
  len=len*10+c; //add as low decimal
  c=rxbuf[14]; //forth digit
  if((c>0x39)||(c<0x30)) break; //must be in rabhe '0'-'9'
  c-=0x30;
  len=len*10+c; //add as low decimal
  break;
 }

 if(len>MAXDATALEN) return; //check maximal len
 rxdata=len;  //set number of bytes of binary data will be received next
}

//----------------------------------------------------------------
//URC after request of GPRS service: AT+CGATT?#0D
void get_att(void) //OK
{
 //+CGATT: 1
 int16_t c=rxbuf[8]-0x30;
 if((c<0)||(c>1)) return;
 gatt_value=c;
}
//----------------------------------------------------------------
//URC after GPRS shutdown AT+CIPSHUT#0D
void get_shut(void) //OK
{
 //SHUT OK
 cipshut_ok();
}
//----------------------------------------------------------------
//URC in extended voice call info mode with incoming number
void get_clip(void)
{
 //+CLIP: "+380958110260",145,"",0,"",0#0D#0A
 int16_t i;
 uint8_t c;

 call_number[0]=0;
 if(rxlen<8) return;
 for(i=8;i<39;i++)
 {
  c=rxbuf[i];
  if(c=='"') c=0;
  call_number[i-8]=c;
 }
 call_number[31]=0;
 state |= STATE_CNUM;
}
//----------------------------------------------------------------
//URC after sending data over socket
//AT+CIPSEND=1,4#0D
//>
//data
void get_send(void) //OK
{
 //1, SEND OK
 //check send ok
 if(rxbuf[8]=='O') cipsend_ok();
 else cipsend_er();
}

//----------------------------------------------------------------
//URC on module start: sim card ready for calls
void get_cr(void)
{
 //Call Ready
 //some start initialisation here
}

//----------------------------------------------------------------
//URC on module start: sim card ready for sms
void get_sr(void) //OK
{
 //SMS Ready
 cmgd_go();  //delete all old SMS
}

//----------------------------------------------------------------
//URC report after sms was send AT+CMGS="+380677812522"#0D
void get_smssend(void)
{
 //+CMGS: 135


 int16_t ret=0;
 int16_t c;

 if(rxbuf[4]!='S') return;

 //obtain sms sending report code to 999
 while(1)
 {
  c=rxbuf[7];  //first digit
  if((c>0x39)||(c<0x30)) break; //must be in rabhe '0'-'9'
  c-=0x30;
  ret=c; //add as low decimal
  c=rxbuf[8]; //second digit
  if((c>0x39)||(c<0x30)) break; //must be in rabhe '0'-'9'
  c-=0x30;
  ret=ret*10+c; //add as low decimal
  c=rxbuf[9];  //third digit
  if((c>0x39)||(c<0x30)) break; //must be in rabhe '0'-'9'
  c-=0x30;
  ret=ret*10+c; //add as low decimal
  break;
 }
 smsrep=ret; //set sms report code
 state |= STATE_SMSD; //set sms delivered flag
}
//----------------------------------------------------------------
//URC reports after call terminated remotely

void get_callstop(void)
{
 //NO CARRIER
 //NO ANSWER
 //BUSY
 state &= (~STATE_CALL); //clear call flag
 call_number[0]=0;
 state &= (~STATE_CNUM);
}

void get_dtmf(void)
{
 if(dtmflen<32) dtmfbuf[dtmflen++]=rxbuf[7];
 state|=STATE_DTMF;
}
