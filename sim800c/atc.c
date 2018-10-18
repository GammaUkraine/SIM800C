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

//===================================================================
//GSM module SIM800C driver
//AT-command functions are executed functiuon and callbacks for:
//answer OK, ERROR and answering timeout occured
//===================================================================

//===================================================================
//check GPRS is avaliable
//AT+CGATT?#0D
//===================================================================
void cgatt_go(void)
{
 strcpy(txbuf, (int8_t*)"AT+CGATT?\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 state |= STATE_BUSY; //clear all, set busy flag
 cb_ok=cgatt_ok;   //set callbacks
 cb_er=cgatt_er;
 cb_to=cgatt_to;
}
//-------------------------------------------------------------------
void cgatt_ok(void)
{
 cipmux_go(); //gprs service avaliable: set multiconnecting mode
}
//-------------------------------------------------------------------
void cgatt_er(void)
{
 state |= STATE_ERROR;
 errcode=10;
 cipshut_go(); //gprs unavaliable: shutdown gprs
}
//-------------------------------------------------------------------
void cgatt_to(void)
{
 state |= STATE_ERROR;
 errcode=11;
 cipshut_go(); //timeout of request: shutdown gprs
}

//===================================================================
//set multiconnecting mode
//AT+CIPMUX=1#0D
//===================================================================
void cipmux_go(void)
{
 if(gatt_value) //check gprs service avaiable
 {
  strcpy(txbuf, (int8_t*)"AT+CIPMUX=1\r");
  txlen=strlen(txbuf);
  tout=tnow+2; //timeout 2 sec for this command
  state |= STATE_BUSY; //clear all, set busy flag
  cb_ok=cipmux_ok;   //set callbacks
  cb_er=cipmux_er;
  cb_to=cipmux_to;
 }
 else cipshut_go();
}
//-------------------------------------------------------------------
void cipmux_ok(void)
{
 cstt_go(); //muticonnecting mode set ok: configure APN
}
//-------------------------------------------------------------------
void cipmux_er(void)
{
 state |= STATE_ERROR;
 errcode=12;
 cipshut_go(); //error multiconnecting: shutdown gprs
}
//-------------------------------------------------------------------
void cipmux_to(void)
{
 state |= STATE_ERROR;
 errcode=13;
 cipshut_go(); //timeout of set multiconnecting: shutdown gprs
}

//===================================================================
//set APN
//AT+CSTT="Internet"#0D
//===================================================================
void cstt_go(void)
{
 strcpy(txbuf, (int8_t*)"AT+CSTT=\"");
 strcpy(txbuf+strlen(txbuf), str);
 strcpy(txbuf+strlen(txbuf), (int8_t*)"\"\r");
 txlen=strlen(txbuf);
 state |= STATE_BUSY; //clear all, set busy flag
 tout=tnow+2; //timeout 2 sec for this command
 cb_ok=cstt_ok;   //set callbacks
 cb_er=cstt_er;
 cb_to=cstt_to;
}
//-------------------------------------------------------------------
void cstt_ok(void)
{
 ciicr_go(); //APN set OK: activate gprs
}
//-------------------------------------------------------------------
void cstt_er(void)
{
 state |= STATE_ERROR;
 errcode=14;
 cipshut_go(); //error set APN: shutdown gprs
}
//-------------------------------------------------------------------
void cstt_to(void)
{
 state |= STATE_ERROR;
 errcode=15;
 cipshut_go(); //timeout of set APN: shutdown gprs
}
//-------------------------------------------------------------------
//===================================================================
//RUN gprs
//AT+CIICR#0D
//===================================================================
void ciicr_go(void)
{
 strcpy(txbuf, (int8_t*)"AT+CIICR\r");
 txlen=strlen(txbuf);
 state |= STATE_BUSY; //clear all, set busy flag
 tout=tnow+GPRS_TIMEOUT; //timeout 85 sec for this command
 cb_ok=ciicr_ok;   //set callbacks
 cb_er=ciicr_er;
 cb_to=ciicr_to;
}
//-------------------------------------------------------------------
void ciicr_ok(void)
{
 cifsr_go(); //gprs activated ok: get ip
}
//-------------------------------------------------------------------
void ciicr_er(void)
{
 state |= STATE_ERROR;
 errcode=16;
 cipshut_go(); //error activate gprs: shutdown gprs
}
//-------------------------------------------------------------------
void ciicr_to(void)
{
 state |= STATE_ERROR;
 errcode=17;
 cipshut_go(); //timeout of activate gprs: shutdown gprs
}

//===================================================================
//get local ip
//AT+CIFSR#0D
//===================================================================
void cifsr_go(void)
{
 strcpy(txbuf, (int8_t*)"AT+CIFSR\r");
 txlen=strlen(txbuf);
 state |= STATE_BUSY; //clear all, set busy flag
 ip[0]=0; //ip request
 tout=tnow+1; //timeout 1 sec for this command
 cb_ok=0;   //set callbacks
 cb_er=cifsr_er;
 cb_to=cifsr_to;
}
//-------------------------------------------------------------------
void cifsr_ok(void)
{
 state &= (~STATE_BUSY); //clear busy state
 state |= STATE_GPRS; //set GPRS activated
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cifsr_er(void)
{
 state |= STATE_ERROR;
 errcode=18;
 cipshut_go(); //error get ip: shutdown gprs
}
//-------------------------------------------------------------------
void cifsr_to(void)
{
 state |= STATE_ERROR;
 errcode=19;
 cipshut_go(); //error get ip: shutdown gprs
}

//===================================================================
//Shutdown gprs
//AT+CIPSHUT#0D
//===================================================================
void cipshut_go(void)
{
 strcpy(txbuf, (int8_t*)"AT+CIPSHUT\r");
 txlen=strlen(txbuf);
 tout=tnow+SHUT_TIMEOUT; //timeout 65 sec for this command
 state &= 0xFF;  //clear all GPRS flags
 state |= STATE_BUSY; //set busy flag
 cb_ok=cipshut_ok;   //set callbacks
 cb_er=cipshut_er;
 cb_to=cipshut_to;
}
//-------------------------------------------------------------------
void cipshut_ok(void)
{
 ip[0]=0xFF; //IP engine disabled
 state &= 0xFF; //GPRS deactivated OK, left only sms and call states
 state &= (~STATE_BUSY);   //clear busy flag
 cb_ok=0;
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipshut_er(void)
{
 state |= STATE_ERROR;
 errcode=20;
 state &= 0xFF; //GPRS deactivated with error
 state &= (~STATE_BUSY);   //clear busy flag
 cb_ok=0;
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipshut_to(void)
{
 state |= STATE_ERROR;
 errcode=21;
 state &= 0xFF; //GPRS deactivated with error
 cb_ok=0;
 cb_er=0;
 cb_to=0;
}

//===================================================================
//TCP listening Server start
//AT+CIPSERVER=1,4444#0D
//===================================================================
void cipserver1_go(void)
{
 if((internal_port)&&(state & STATE_GPRS))
 {
  strcpy(txbuf, (int8_t*)"AT+CIPSERVER=1,");
  itoa(internal_port, txbuf+strlen(txbuf), 10);
  strcpy(txbuf+strlen(txbuf), (int8_t*)"\r");
  txlen=strlen(txbuf);
  tout=tnow+2; //timeout 2 sec for this command
  state |= STATE_BUSY; //clear all, set busy flag
  cb_ok=0;   //set callbacks
  cb_er=cipserver1_er;
  cb_to=cipserver1_to;
 }
 else cipserver0_go();
}
//-------------------------------------------------------------------
void cipserver1_ok(void)
{
 state &= (~STATE_BUSY); //server start ok: clear busy flag
 state |= STATE_SRV;
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipserver1_er(void)
{
 state |= STATE_ERROR;
 errcode=22;
 state &= (~STATE_BUSY); //server start error: clear busy flag
 state &= (~STATE_SRV);
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipserver1_to(void)
{
 state |= STATE_ERROR;
 errcode=23;
 cipshut_go(); //timeout of start server: shutdown gprs
}

//===================================================================
//TCP listening Server stop
//AT+CIPSERVER=0#0D
//===================================================================
void cipserver0_go(void)
{
  strcpy(txbuf, (int8_t*)"AT+CIPSERVER=0\r");
  txlen=strlen(txbuf);
  tout=tnow+2; //timeout 2 sec for this command
  state |= STATE_BUSY; //clear all, set busy flag
  cb_ok=0;   //set callbacks
  cb_er=cipserver0_er;
  cb_to=cipserver0_to;
}
//-------------------------------------------------------------------
void cipserver0_ok(void)
{
 state &= (~STATE_BUSY); //server stop ok: clear busy flag
 state &= (~STATE_SRV);
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipserver0_er(void)
{
 state |= STATE_ERROR;
 errcode=24;
 state &= (~STATE_BUSY); //server stop error: clear busy flag
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipserver0_to(void)
{
 state |= STATE_ERROR;
 errcode=25;
 cipshut_go(); //timeout of start server: shutdown gprs
}

//===================================================================
//connect socket to remote peer
//AT+CIPSTART=1,"TCP","94.45.70.111","3333"#0D
//===================================================================
void cipstart_go(void)
{
 if(!(state & STATE_GPRS)) return;
 if(internal_port && str[0])
 {
  strcpy(txbuf, (int8_t*)"AT+CIPSTART=");
  itoa(internal_ch, txbuf+strlen(txbuf), 10);
  if(internal_tcp) strcpy(txbuf+strlen(txbuf), (int8_t*)",\"TCP\",\"");
  else strcpy(txbuf+strlen(txbuf), (int8_t*)",\"UDP\",\"");
  strcpy(txbuf+strlen(txbuf), str);
  strcpy(txbuf+strlen(txbuf), (int8_t*)"\",\"");
  itoa(internal_port, txbuf+strlen(txbuf), 10);
  strcpy(txbuf+strlen(txbuf), (int8_t*)"\"\r");
  txlen=strlen(txbuf);
  tout=tnow+CONNECT_TIMEOUT; //timeout 75 sec for this command
  state |= STATE_BUSY; //clear all, set busy flag
  cb_ok=0;   //set callbacks
  cb_er=cipstart_er;
  cb_to=cipstart_to;
 }
 else cipclose_go();
}
//-------------------------------------------------------------------
void cipstart_ok(void)
{
   //OK after command
   state &= (~STATE_BUSY);  //error of connect
   cb_ok=0;   //set callbacks
   cb_er=0;
   cb_to=0;
}
//-------------------------------------------------------------------
void cipstart_er(void)
{
 state |= STATE_ERROR;
 errcode=26;
 state &= (~STATE_BUSY);  //error of connect
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipstart_to(void)
{
  state |= STATE_ERROR;
  errcode=27;
  cipshut_go(); //timeout of connect: shutdown gprs
}

//===================================================================
//disconnect socket
//AT+CIPCLOSE=1#0D
//===================================================================
void cipclose_go(void)
{
  strcpy(txbuf, (int8_t*)"AT+CIPCLOSE=");
  itoa(internal_ch, txbuf+strlen(txbuf), 10);
  strcpy(txbuf+strlen(txbuf), (int8_t*)"\r");
  txlen=strlen(txbuf);
  tout=tnow+CLOSE_TIMEOUT; //timeout 2 sec for this command, but we use 10 instead
  state |= STATE_BUSY; //clear all, set busy flag
  cb_ok=cipclose_ok;   //set callbacks
  cb_er=cipclose_er;
  cb_to=cipclose_to;
}
//-------------------------------------------------------------------
void cipclose_ok(void)
{
 //no to do: OK after answer
 state &= (~STATE_BUSY);  //error of closing
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipclose_er(void)
{
 state |= STATE_ERROR;
 errcode=28;
 state &= (~STATE_BUSY);  //error of closing
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipclose_to(void)
{
 state |= STATE_ERROR;
 errcode=29;
 cipshut_go(); //timeout of closing: shutdown gprs
}

//===================================================================
//send data over socket
//AT+CIPSEND=1,4#0D
//>
//data
//===================================================================
void cipsend_go(void)
{
  strcpy(txbuf, (int8_t*)"AT+CIPSEND=");
  itoa(internal_ch, txbuf+strlen(txbuf), 10);
  strcpy(txbuf+strlen(txbuf), (int8_t*)",");
  itoa(internal_len, txbuf+strlen(txbuf), 10);
  strcpy(txbuf+strlen(txbuf), (int8_t*)"\r");
  txlen=strlen(txbuf);
  tout=tnow+60; //timeout 645 sec for this command, but we use more int16_t
  state |= STATE_BUSY; //clear all, set busy flag
  state &= (~STATE_SEND); //clear send compleet flag
  insend=1;
  cb_ok=0;   //set callbacks
  cb_er=cipsend_er;
  cb_to=cipsend_to;
}
//-------------------------------------------------------------------
void cipsend_ok(void)
{
 insend=0; //clear sending flag
 state &= (~STATE_BUSY);  //sended
 state |= STATE_SEND;   //set send compleet flag
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipsend_er(void)
{
 state |= STATE_ERROR;
 errcode=30;
 insend=0; //clear sending flag
 state &= (~STATE_BUSY);  //error of sending
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}
//-------------------------------------------------------------------
void cipsend_to(void)
{
  state |= STATE_ERROR;
  errcode=31;
  insend=0; //clear sending flag
  cipclose_go();
}

//===================================================================
//delete all sms
//AT+CMGDA="DEL ALL"#0D
//===================================================================
void cmgd_go(void)
{
 strcpy(txbuf, (int8_t*)"AT+CMGDA=\"DEL ALL\"\r");
 txlen=strlen(txbuf);
 tout=tnow+10; //timeout 10 sec for this command
 state |= STATE_BUSY; //clear all, set busy flag
 cb_ok=cmgd_ok;   //set callbacks
 cb_er=skip_err;
 cb_to=skip_err;
}

void cmgd_ok(void)
{
 state &= (~STATE_BUSY);
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}

//-------------------------------------------------------------------
//clear busy flag
void skip_err(void)
{
 state |= STATE_ERROR;
 errcode=32;
 state &= (~STATE_BUSY);
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}

//===================================================================
//send sms
//AT+CMGS="+380677812522"#0D
//#0D#0A>
//test#1A
void smssend_go(void)
{
  //AT+CMGS="+380677812522"#0D
  strcpy(txbuf, (int8_t*)"AT+CMGS=\"");
  strcpy(txbuf+strlen(txbuf), (int8_t*)sms_number);
  strcpy(txbuf+strlen(txbuf), (int8_t*)"\"\r");
  txlen=strlen(txbuf);
  tout=tnow+2; //timeout 2 sec for this command
  state &= (~STATE_SMSD); //clear delivering flag
  state |= STATE_BUSY; //clear all, set busy flag
  insend=1;
  cb_ok=smssend_ok;   //set callbacks
  cb_er=cipsend_er;
  cb_to=cipsend_to;
}

void smssend_ok(void)
{
 insend=0; //clear sending flag
 state &= (~STATE_BUSY);  //error of sending
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}

void smssend_er(void)
{
 state |= STATE_ERROR;
 errcode=33;
 insend=0; //clear sending flag
 state &= (~STATE_BUSY);  //error of sending
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}

void smssend_to(void)
{
 state |= STATE_ERROR;
 errcode=34;
 insend=0; //clear sending flag
 state &= (~STATE_BUSY);  //error of sending
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}

//==============================================
void atd_go(void)
{
 strcpy(txbuf, (int8_t*)"ATD");
 strcpy(txbuf+strlen(txbuf), (int8_t*)call_number);
 strcpy(txbuf+strlen(txbuf), (int8_t*)";\r");
 txlen=strlen(txbuf);
 tout=tnow+5; //timeout 2 sec for this command
 state |= STATE_BUSY; //clear all, set busy flag
 cb_ok=call_ok;   //set callbacks
 cb_er=call_err;
 cb_to=call_to;
}

void ata_go(void)
{
 strcpy(txbuf, (int8_t*)"ATA\r");
 txlen=strlen(txbuf);
 tout=tnow+5; //timeout 2 sec for this command
 state |= STATE_BUSY; //clear all, set busy flag
 cb_ok=call_ok;   //set callbacks
 cb_er=call_err;
 cb_to=call_to;
}

void ath_go(void)
{
 strcpy(txbuf, (int8_t*)"ATH\r");
 txlen=strlen(txbuf);
 tout=tnow+5; //timeout 2 sec for this command
 state |= STATE_BUSY; //clear all, set busy flag
 cb_ok=call_out;   //set callbacks
 cb_er=call_err;
 cb_to=call_to;
}

void call_ok(void)
{
 state &= (~STATE_BUSY);  //error of sending
 state |= STATE_CALL;
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}

void call_out(void)
{
 state &= (~STATE_BUSY);  //error of sending
 state &= (~STATE_CALL);
 state &= (~STATE_CNUM);
 call_number[0]=0;
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;

}

void call_err(void)
{
 state |= STATE_ERROR;
 errcode=35;
 state &= (~STATE_BUSY);  //error of sending
 state &= (~STATE_CALL);
 state &= (~STATE_CNUM);
 call_number[0]=0;
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}

void call_to(void)
{
 state |= STATE_ERROR;
 errcode=36;
 state &= (~STATE_BUSY);  //error of sending
 state &= (~STATE_CALL);
 state &= (~STATE_CNUM);
 call_number[0]=0;
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}

//==============================================
void dtmf_go(void)
{
 int16_t i, p;
 if(!dtmflen) return;
 strcpy(txbuf, (int8_t*)"AT+VTS=\"");
 p=strlen(txbuf);
 for(i=0;i<dtmflen;i++)
 {
  txbuf[p++]=dtmfbuf[i];
  txbuf[p++]=',';
 }
 txbuf[--p]=0;
 strcpy(txbuf+strlen(txbuf), (int8_t*)"\"\r");
 txlen=strlen(txbuf);
 tout=tnow+5+2*dtmflen; //timeout n sec for this command
 dtmflen=0;
 state |= STATE_BUSY; //clear all, set busy flag
 cb_ok=dtmf_ok;   //set callbacks
 cb_er=dtmf_er;
 cb_to=dtmf_to;
}

void dtmf_ok(void)
{
 state &= (~STATE_BUSY);
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}


void dtmf_er(void)
{
 state |= STATE_ERROR;
 errcode=37;
 state &= (~STATE_BUSY);
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}


void dtmf_to(void)
{
 state |= STATE_ERROR;
 errcode=38;
 state &= (~STATE_BUSY);
 cb_ok=0;   //set callbacks
 cb_er=0;
 cb_to=0;
}


