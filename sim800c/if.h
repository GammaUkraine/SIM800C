//==================================================================
//                 interface of SIM800C driver
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



#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//==================================================================
//                        special procedures
//==================================================================
void rx(int8_t* data, int16_t len);  //add data received over uart (safe-thread)
uint32_t poll(uint32_t time); //main driver's task, returns state
//------------------------------------------------------------------

//==================================================================
//                       reading procedures
//==================================================================
int16_t tx(int8_t* data);  //get data for transmitt over uart (flag  STATE_TX    0x10)
int16_t readdata(uint8_t* data, uint8_t* ch); //read received data, returns len (flag STATE_DATA  0x4000)
int16_t readsms(uint8_t* data, uint8_t* phone); //read receved text SMS (flag STATE_SMS   0x20)
int16_t readdtmf(uint8_t* data); //read received DTMF tone (flag STATE_DTMF  0x40)
int16_t smsd(void); //read SMS delivery report (flag STATE_SMSD  0x80)
void callnum(uint8_t* num); //read incoming call number (flag STATE_CNUM  0x100)
int16_t lasterr(void); //read error code (flag STATE_ERROR 0x01)
//------------------------------------------------------------------

//==================================================================
//                            commands
//can be execute only if NOT flag: STATE_BUSY  0x02
//==================================================================
void reinit(void); //manually init SIM800 module
void gprs(int8_t* apn); //activate/deactivate gprs  (flag STATE_GPRS  0x1000)
void listen(uint16_t port); //run/stop tcp listener (flag STATE_SRV   0x2000)
void connect(uint8_t ch, uint8_t tcp, int8_t* dest, uint16_t port); //connect to remote tcp or udp server (flags STATE_CH0 - STATE_CH4)
void close(uint8_t ch); //close tcp or udp connecting (flags STATE_CH0 - STATE_CH4)
void send(uint8_t ch, int8_t* data, uint16_t len); //send data over tcp or udp connecting (flag STATE_SEND  0x8000)
void sendsms(uint8_t* phone, int8_t* data); //send text sms
void delallsms(void); //delete all received sms
void call(int8_t* num);  //make outgoing call (flag STATE_CALL)
void answer(void);  //answer to incoming call (flag STATE_CALL)
void reject(void);  //reject unanswered incoming call or terminate active incoming or outgoing call (flag STATE_CALL)
void senddtmf(uint8_t* data, int16_t len); //send dtmf tones (flag STATE_CALL)
//------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
