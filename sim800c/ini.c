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

#include "def.h"
#include "data.h"
#include "ini.h"


//===================================================================
//GSM module SIM800C driver
//First module initilization and storing in non-volatile
//memory using AT&W
//===================================================================

//===================================================================
//Manually start SIM800 initialization
//===================================================================
void creini_go(void)   //AT+CREG=0
{
 strcpy(txbuf, (char*)"AT+CREG=0\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 cb_ok=creg_ok;   //set callbacks
 cb_er=creini_er;
 cb_to=creini_to;
}

void creini_ok(void)
{
 creq_go();
}

void creini_er(void)
{
 state &= (~STATE_BUSY);
 state |= STATE_ERROR;
 errcode=40;
}

void creini_to(void)
{
 state &= (~STATE_BUSY);
 state |= STATE_ERROR;
 errcode=41;
}

//===================================================================
//Check is module initilized and registered in GSM network
//===================================================================
 void creq_go(void)
 {
  strcpy(txbuf, (char*)"AT+CREG?\r");
  txlen=strlen(txbuf);
  tout=tnow+2; //timeout 2 sec for this command
  state |= STATE_BUSY; 
  cb_ok=creq_ok;   //set callbacks
  cb_er=creq_to;
  cb_to=creq_to;
 }

//-------------------------------------------------------------------
 void creq_ok(void)
 {
  cb_ok=0;   //set callbacks
  cb_er=0;
  cb_to=0;
  state &= (~STATE_BUSY);
  if(!(state&STATE_INIT)) //check initilization flag set in parser of +CREG:
  {    //module not initialized yet
   state |= STATE_ERROR;
   errcode=1;
   ini_go(); //provide initializing sequence
  }
 }

 //-------------------------------------------------------------------
 void creq_to(void)
 {
  state |= STATE_ERROR;
  errcode=2;
  creq_go();
 }





//===================================================================
//Start of initializing sequence:
// disable At echo: ATE0
//===================================================================
void ini_go(void)   //ATE0 disable AT command echo
{
 //ATE0#0D
 strcpy(txbuf, (char*)"ATE0\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 state |= STATE_BUSY; //clear all, set busy flag
 cb_ok=ini_ok;   //set callbacks
 cb_er=ini_er;
 cb_to=ini_er;
}

void ini_ok(void)
{
 ipr_go();
}

void ini_er(void)
{
 state |= STATE_ERROR;
 ini_go();
}

//--------------------------------------------------
void ipr_go(void)   //AT+IPR=115200
{
 strcpy(txbuf, (char*)"AT+IPR=115200\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 cb_ok=ipr_ok;   //set callbacks
 errcode=4;
}

void ipr_ok(void)
{
 creg_go();
}

//--------------------------------------------------
void creg_go(void)   //AT+CREG=1
{
 strcpy(txbuf, (char*)"AT+CREG=1\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 cb_ok=creg_ok;   //set callbacks
 errcode=5;
}

void creg_ok(void)
{
 cmgf_go();
}

//--------------------------------------------------
void cmgf_go(void)   //AT+CMGF=1
{
 strcpy(txbuf, (char*)"AT+CMGF=1\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 cb_ok=cmgf_ok;   //set callbacks
 errcode=6;
}

void cmgf_ok(void)
{
 cnmi_go();
}

//--------------------------------------------------
void cnmi_go(void)   //AT+CMNI=2,2,0,0,0
{
 strcpy(txbuf, (char*)"AT+CNMI=2,2,0,0,0\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 cb_ok=cnmi_ok;   //set callbacks
 errcode=7;
}

void cnmi_ok(void)
{
 ddet_go();
}

//--------------------------------------------------
void ddet_go(void) //AT+DDET=1,1000,0,0
{
 strcpy(txbuf, (char*)"AT+DDET=1,1000,0,0\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 cb_ok=ddet_ok;   //set callbacks
 errcode=8;
}

void ddet_ok(void)
{
 clip_go();
}



//--------------------------------------------------
void clip_go(void) //AT+CLIP=1
{
 strcpy(txbuf, (char*)"AT+CLIP=1\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 cb_ok=clip_ok;   //set callbacks
 errcode=39;
}

void clip_ok(void)
{
 atw_go();
}

//--------------------------------------------------
void atw_go(void)   //AT&W
{
 strcpy(txbuf, (char*)"AT&W\r");
 txlen=strlen(txbuf);
 tout=tnow+2; //timeout 2 sec for this command
 cb_ok=atw_ok;   //set callbacks
 errcode=9;
}

void atw_ok(void)
{
 creq_go();
}






