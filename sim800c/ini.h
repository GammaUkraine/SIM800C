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



void creini_go(void);  //AT+CREG=0
void creini_ok(void);
void creini_er(void);
void creini_to(void);

void creq_go(void); //AT+CREG?
void creq_ok(void);
void creq_to(void);

void ini_go(void);   //ATE0
void ini_ok(void);
void ini_er(void);

void ipr_go(void);   //AT+IPR=115200
void ipr_ok(void);

void creg_go(void);   //AT+CREG=1
void creg_ok(void);

void cmgf_go(void);   //AT+CMGF=1
void cmgf_ok(void);

void cnmi_go(void);   //AT+CMNI=2,2,0,0,0
void cnmi_ok(void);

void ddet_go(void);   //AT+DDET=1,1000,0,0
void ddet_ok(void);

void clip_go(void); //AT+CLIP=1
void clip_ok(void);

void atw_go(void);   //AT&W
void atw_ok(void);


