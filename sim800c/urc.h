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



void rxprocess(int16_t len); //parce URC  received over uart
void data_send(void); //outpu binary data for sending after promt >

void get_ip(void);
void get_srv(void);
void get_accept(void);
void get_connect(void);
void get_close(void);
void get_reg(void);
void get_sms(void);
void get_call(void);
void get_data(void);
void get_att(void);
void get_shut(void);
void get_clip(void);
void get_send(void);
void get_cr(void);
void get_sr(void);
void get_smssend(void);
void get_callstop(void);
void get_dtmf(void);

