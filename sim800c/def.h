//==================================================================
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



//                 idefinitions of SIM800C driver
//==================================================================

//maximum data len
#define MAXDATALEN 576+32   //GPRS binary data (MTU)
#define MAXSMSLEN 160+4     //SMS text data

//timeouts in sec
#define GPRS_TIMEOUT     85  //GPRS activation
#define SHUT_TIMEOUT     65  //GPRS deactivation
#define CONNECT_TIMEOUT  75  //TCP socket connection
#define CLOSE_TIMEOUT    10  //TCP socket disconnection


//==================================================================
//                        binary FLAGS of state
//==================================================================

#define STATE_ERROR 0x01   //Driver's error
#define STATE_BUSY  0x02    //Command in progress
#define STATE_REG   0x04     //GSM registration
#define STATE_CALL  0x08   //Voice call is active

#define STATE_TX    0x10      //TX UART data ready
#define STATE_SMS   0x20    //Incoming SMS received
#define STATE_DTMF  0x40   //DTMF data received
#define STATE_SMSD  0x80  //SMS delivery report received

#define STATE_CNUM  0x100 //Incoming call number received
#define STATE_INIT  0x200 //Module initialized

//gprs
#define STATE_GPRS  0x1000  //GPRS is active
#define STATE_SRV   0x2000   //TCP server is active
#define STATE_DATA  0x4000  //GPRS data received
#define STATE_SEND  0x8000  //GPRS data send compleet

//flag of sockets in use   (only sockets 0-4 can be used, other are reserved)
#define STATE_CH0 0x010000
#define STATE_CH1 0x020000
#define STATE_CH2 0x040000
#define STATE_CH3 0x080000
#define STATE_CH4 0x100000
#define STATE_CH5 0x200000
#define STATE_CH6 0x400000
#define STATE_CH7 0x800000
