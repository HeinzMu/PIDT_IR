#include <Arduino.h>
#include "HW_TRICYCLE_V2001.h"
#if SW_DEBUG
#include <avr8-stub.h>
#include <SW_Debug.h>
#endif
#include <PIDT_IR.h>
#include <TimerOne.h>

#define USER_MOTOR MY_ELEGO_GM
#include <Motors.h>
#include "cmdPIDT_IR.h"

#define BAUDRATE    115200							// Baudrate definieren
#define SER_TIMEOUT 30000							// in mSec
#define REGLER_TYP  PID_REGLER
#define CLI4_MAXBUF	32

#define T3PWM_15600   _BV(CS30)
#define T3PWM_1950    _BV(CS31)
#define T3PWM_244     _BV(CS31) | _BV(CS30)
#define T3PWM_61      _BV(CS32)
#define T3PWM_15      _BV(CS32) | _BV(CS30)

extern PIDT_IR Regler_R, Regler_L;
extern int CmdLineProcess(char *);
extern int help();

char cBuf[CLI4_MAXBUF];
static byte iBuf = 0;
txtCmd tCmd;

void initFastPWM(void){
  
  // enable waveform generation for Timer 3 (OC3A and OC3B)
  // COM3A1 = 1,  COM3A0 = 1 -> clear OC3A on compare match an set it when Timer 3 at bottom (=0)
  // WGM32 =  1,  WGM31  = 1,  WGM30 = 1  -> set to fast PWM and 10 bit resolution
  TCCR3A = _BV(COM3A1) | _BV(COM3B1) | _BV(COM3C1) | _BV(WGM31) | _BV(WGM30);
  TCCR3B = _BV(WGM32)  | T3PWM_244;
  TCCR3C = _BV(FOC3B)  | _BV(FOC3C);  // enable OC3B and OC3C
  DDRE |= _BV(PE4);                   // set OC3B to output Pin 2
  DDRE |= _BV(PE5);                   // set OC3C to output Pin 3
}

void processRegler(void){
	Regler_R.setReglerIR();
	Regler_L.setReglerIR();
}

int checkCmd(){
	int CmdStatus = CmdLineProcess((char *)cBuf);
	switch (CmdStatus){
		case CMD_NDEF:
			SePLN(tCmd.NDEF_TXT);
		break;
		case CMD_OK:
			SePLN(tCmd.OK_TXT);
		break;
		case CMD_BAD_CMD:
			SePLN(tCmd.BAD_CMD_TXT);
		break;
		case CMD_TOO_MANY_ARGS:
			SePLN(tCmd.TOO_MANY_ARGS_TXT);
		break;
		case CMD_TOO_FEW_ARGS:
			SePLN(tCmd.TOO_FEW_ARGS_TXT);
		break;
		case CMD_WRONG_ARG:
			SePLN(tCmd.WRONG_ARG_TXT);
		break;
		case CMD_PORT_ERR:
			SePLN(tCmd.PORT_ERR_TXT);
		break;
		default:
			SePLN(tCmd.NDEF_TXT);
		break;
	}
	return (CmdStatus);
}

void mySerEvent(){

	int ch;
	static bool bP = false;

	if (MY_SERIAL.available() > 0){
		ch = MY_SERIAL.read();
		MY_SERIAL.write(ch);
		if (iBuf < MAXBUF){
			cBuf[iBuf++] = (char)ch;
		}
		if (ch == '\r'){
			cBuf[--iBuf] = 0;
			iBuf = 0;
			SePLN("\r\n>");
			checkCmd();
			memset(cBuf, 0, sizeof(cBuf));
			bP = false;
		}
	}else{
		if (!bP){
			SeP(F(">"));
			bP = true;
		}
	}
}

void setup() {

#if (SW_DEBUG)
	debug_init();
	pinMode(SF_IN, INPUT_PULLUP);
#endif

	initFastPWM();

#if !SW_DEBUG
 	//
	// Timer 1 initialisieren
	//
	Timer1.initialize(TIMER1_TIME);											// TIMER1_TIME ist in PIDT_IR.h definiert
	Timer1.attachInterrupt(processRegler);
#endif

	Regler_R.Init(REGLER_TYP, INC_R, INC_R_MON, PWM_OUTR, THIS_ISR(Regler_R), RISING, LEFT_MR, RIGHT_MR, 15000);
	Regler_L.Init(REGLER_TYP, INC_L, INC_L_MON, PWM_OUTL, THIS_ISR(Regler_L), RISING, LEFT_ML, RIGHT_ML, 15000);

	MY_SERIAL.begin(BAUDRATE);
	MY_SERIAL.setTimeout(SER_TIMEOUT);
	help(0, NULL);
}

void loop() {

	mySerEvent();
#if SW_DEBUG
	processRegler();
#endif
}
