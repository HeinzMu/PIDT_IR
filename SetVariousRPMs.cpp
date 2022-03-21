/*********************************************************************
 * 
 * Bevor das Programm im Debugmodus ausgefürt wird, muss in der
 * "Debug.h" in Zeile 11 der 
 * #define SERIAL_COM	3 auf
 * #define SERIAL_COM	0 umgestelt werden.
 * 
 * Dadurch werden die seriellen Ausgaben unterdrückt
 * 
 * 
 ********************************************************************/
#include <arduino.h>
#include <debug.h>
#if SW_DEBUG
#include <avr8-stub.h>
#include <SW_Debug.h>
#endif
#include "..\examples\HW_TRICYCLE_V2001.h"
#include <PIDT_IR.h>
#include <TimerOne.h>
#include <SoftWatchDog.h>

#define USER_MOTOR MY_ELEGO_GM
#include <Motors.h>

#define BAUDRATE    115200
#define SER_TIMEOUT 1
#define REGLER_TYP  PID_REGLER
#define TACHO_FILT  5
#define TACHO_MAX_N USER_MOTOR_N_MAX * 1.1

#define T3PWM_15600   _BV(CS30)
#define T3PWM_1950    _BV(CS31)
#define T3PWM_244     _BV(CS31) | _BV(CS30)
#define T3PWM_61      _BV(CS32)
#define T3PWM_15      _BV(CS32) | _BV(CS30)

PIDT_IR Regler_R, Regler_L;

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


void setup() {

#if (SW_DEBUG)
	debug_init();
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
	MY_SERIAL.setTimeout(10);
}

void loop() {

static int i = 0;
#if SW_DEBUG
const uint32_t uiAccel = 100000L;
#else
const uint32_t uiAccel = 2000L;
#endif
char c[128];
uint32_t ui_iAccel = 0L;
WATCH W1;

	for (i = 0; i < 10; i++){
		SePLN();
		ui_iAccel = i * uiAccel;
#if SW_DEBUG
		breakpoint();
#endif
		Regler_R.setAccel(ui_iAccel);
		Regler_L.setAccel(ui_iAccel);
		SeP3LN("Accel_", i, ": ", ui_iAccel);
		sprintf(c, "R:Soll SollAcc IstSped   L:Soll SollAcc IstSped");
		SePLN(c);
		Regler_R.setSpeed(5400L);
		do{
#if SW_DEBUG
			processRegler();
#endif
			sprintf(c, "R:%4d\t%7d\t%7d\tL:%4d\t%7d\t%7d", 5400, (int)Regler_R.get_lvSollAcc(), (int)Regler_R.getIstSpeed(), 0, (int)Regler_L.get_lvSollAcc(), (int)Regler_L.getIstSpeed());
			SePLN(c);
			delay(100);
		}while (Regler_R.getSpeedStat());
		Regler_R.setSpeed(0L);
		do{
#if SW_DEBUG
			processRegler();
#endif
			sprintf(c, "R:%4d\t%7d\t%7d\tL:%4d\t%7d\t%7d", 0, (int)Regler_R.get_lvSollAcc(), (int)Regler_R.getIstSpeed(), 0, (int)Regler_L.get_lvSollAcc(), (int)Regler_L.getIstSpeed());
			SePLN(c);
			delay(100);
		}while (Regler_R.getSpeedStat());
		Regler_L.setSpeed(5400L);
		do{
#if SW_DEBUG
			processRegler();
#endif
			sprintf(c, "R:%4d\t%7d\t%7d\tL:%4d\t%7d\t%7d", 0, (int)Regler_R.get_lvSollAcc(), (int)Regler_R.getIstSpeed(), 5400, (int)Regler_L.get_lvSollAcc(), (int)Regler_L.getIstSpeed());
			SePLN(c);
			delay(100);
		}while (Regler_L.getSpeedStat());
		Regler_L.setSpeed(0L);
		do{
#if SW_DEBUG
			processRegler();
#endif
			sprintf(c, "R:%4d\t%7d\t%7d\tL:%4d\t%7d\t%7d", 0, (int)Regler_R.get_lvSollAcc(), (int)Regler_R.getIstSpeed(), 0, (int)Regler_L.get_lvSollAcc(), (int)Regler_L.getIstSpeed());
			SePLN(c);
			delay(100);
		}while (Regler_L.getSpeedStat());
	}
	while (true);
}

