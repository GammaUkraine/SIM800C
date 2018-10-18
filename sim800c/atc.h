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
//-------------------------


//send and callback functions for used AT command

void cgatt_go(void);
void cgatt_ok(void);
void cgatt_er(void);
void cgatt_to(void);

void cipmux_go(void);
void cipmux_ok(void);
void cipmux_er(void);
void cipmux_to(void);

void cstt_go(void);
void cstt_ok(void);
void cstt_er(void);
void cstt_to(void);

void ciicr_go(void);
void ciicr_ok(void);
void ciicr_er(void);
void ciicr_to(void);

void cifsr_go(void);
void cifsr_ok(void);
void cifsr_er(void);
void cifsr_to(void);

void cipshut_go(void);
void cipshut_ok(void);
void cipshut_er(void);
void cipshut_to(void);

void cipserver1_go(void);
void cipserver1_ok(void);
void cipserver1_er(void);
void cipserver1_to(void);

void cipserver0_go(void);
void cipserver0_ok(void);
void cipserver0_er(void);
void cipserver0_to(void);

void cipstart_go(void);
void cipstart_ok(void);
void cipstart_er(void);
void cipstart_to(void);

void cipclose_go(void);
void cipclose_ok(void);
void cipclose_er(void);
void cipclose_to(void);

void cipsend_go(void);
void cipsend_ok(void);
void cipsend_er(void);
void cipsend_to(void);

void smssend_go(void);
void smssend_ok(void);
void smssend_er(void);
void smssend_to(void);

void cmgd_go(void);
void cmgd_ok(void);
void skip_err(void);

void atd_go(void);
void ata_go(void);
void ath_go(void);
void call_ok(void);
void call_out(void);
void call_err(void);
void call_to(void);


void dtmf_go(void);
void dtmf_ok(void);
void dtmf_er(void);
void dtmf_to(void);
