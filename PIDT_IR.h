#ifndef PIDT_REGLER_H_
#define PIDT_REGLER_H_

/*
 * PID_Regler.h
 *
 *  Created on: 07.02.2019
 *      Author: Heinz Müller
 */

#include <Arduino.h>
#include "Motors.h"
#include <debug.h>

#define THIS_ISR(a)  []{a.getTacho();}

#define USPMIN 60000000			// µSec / minute
#define MAXBUF 64

#define FORWARD		1
#define BACKWARD	-1
#define TIMER1_TIME	5000		// in µSec
#define TA 1					// Anzahl Reglungen pro Sekunde
//#define NT_MINUS	(1.0E6 / (USER_MOTOR_N_MAX / USER_MOTOR_Inc_p_n / 2.0))
typedef enum eReglerTyp{
REGLER_DEF	=	0,
P_REGLER,
I_REGLER,
D_REGLER,
PI_REGLER,
PD_REGLER,
ID_REGLER,
PID_REGLER,
PT1_REGLER,
PT1_REGLER_S,
PT1_REGLER_T
}tReglerTyp;

#define ACC_DIS		0
#define ACC_ACC		1
#define ACC_DEC		2

typedef struct{
	volatile long lSoll = 0;		        		// Drehzahlvorgabe in 1/min
	volatile long lLastlSoll = 0;					// letzte Soll-Geschwindigkeit
	volatile float fIst_x = 0;           			// aktuelle Regelgröße ( immer >= 0) (IstDrehzahl; erster Wert = 0)
	volatile float fSoll_w = 0;          			// Führungsgröße (immer >= 0) (Sollwert)
	volatile float fAbw_e = 0;              		// Regelabweichung
	volatile float fesum = 0;						// Summation I-Regler
	volatile float fTa = 0;							//
	volatile float fStell_y = 0;					// Stellgröße y
	volatile float yp = 0;							// P-Regelanteil
	volatile float yi = 0;							// I-Regelanteil
	volatile float yd = 0;							// D-Regelanteil
	volatile float ypt1 = 0;						// PT1-Regelanteil
	volatile float y = 0;							// PID-Regelanteil
	volatile int iPWM = 0;							// PWM-Ausgabe des Reglers
	float fkp = USER_MOTOR_kp;						// KP Anteil P-Regler
	float fki = USER_MOTOR_ki;  					// KI Anteil I-Regler
	float fkd = USER_MOTOR_kd;  					// KD Anteil D-Regler
	float fKs = USER_MOTOR_Ks;  					// Verstärkungsfaktor[1/min] PT1 Glieds
	float fTs = USER_MOTOR_Ts;						// Zeitkonstante[sec] des PT1 Glieds
	char selRegler = USER_MOTOR_selRegler;			// PID-Regler ist standard
	volatile unsigned long tStart = 0;				// StartZeit der ISR-Routine
	volatile unsigned long tEnde = 0;				// EndeZeit der ISR-Routine
	volatile unsigned long ltDif = 0;				// Differenz von Ende - Start
	volatile unsigned long ulRestartMotor = 0;		// Zeit für Restart des Motors
	volatile int iDir = FORWARD;
	byte bymyTaskID;								// Instanz-Nummer
} regVars;

typedef struct{
	volatile long lSollAccel = 0;					// Soll-Beschleunigung in 1/(min *sec)
	volatile long lvStartAcc = 0;					// Startgeschwindigkeit für die Beschleunigungsrampe in 1/min
	volatile long lvSollAcc = 0;					// berechnete Sollbeschleunigung
	volatile int iAccCnt = 0;						// Beschleunigungs-Counter
	volatile int iAccDir = ACC_DIS;					// steuerung der Beschleunigungsrampe inaktiv, accel, decel
	volatile bool bAccActive = false;				// Flag für Beschleunigungsrampe aktiv (true) oder fertig (false)
} contVars;


class PIDT_IR{
	public:
		PIDT_IR(){bymyTaskID++;}
		~PIDT_IR(){bymyTaskID--;}
		void Init(tReglerTyp ReglerTyp, int iTacho_pin, int iTmon_pin, int iPWM_pin, void (*ISR_callback)(void), byte byMode = FALLING, int iOutLeft = 0, int iOutRight = 0, unsigned long ulRestartMotor = 200000L);
		float getRegelParam(tReglerTyp RegelParam = REGLER_DEF);
		void setRegelParam(tReglerTyp ReglerTyp, float fParam);
		void setRegelParam(tReglerTyp ReglerTyp, float fParam1, float fParam2);
		unsigned long setRegler(tReglerTyp ReglerTyp, long lSoll, unsigned long ulStart);
		void setReglerIR(void);
		void setSpeed(long lSoll);
		long getSollSpeed();
		float getIstSpeed();
		void setAccel(long lSollAccel);
		bool getSpeedStat(void);
		regVars* get_gVars(void);
		long get_lvSollAcc();
		int get_AccCnt(void);
		float get_aktAbw(void);
		byte get_taskCnt(void);
		void getTacho(void);
		
		regVars _rVars;
		contVars _cVars;
		volatile unsigned long _ulTacho;
		volatile unsigned long _ulTachoCnt;
		volatile bool _bTacho;
		bool bAccel = false;
		unsigned long _ulLastChange;

		
	private:
		byte _byIntMode;
		static byte bymyTaskID;
		int _iTacho_pin, _iPWM_pin, _iOutLeft, _iOutRight;
		int _iTmon_pin;
//		regVars _rVars;
//		contVars _cVars;
		float _fAbw_ealt;        // Regelabweichung
		float _ypt1_alt;
		bool _bTurnBI;
		bool _bTtoggle;

		float _readTacho(void);
		float _calc_P(void);
		float _calc_I(void);
		float _calc_D(void);
		float _calc_PT1(void);
		void _calcRegler(void);
		void _calcPWM(void);
		void _SetDirAndPWM(void);
		void _processRamp(void);
		void _disableRamp(void);
};

#endif /* PIDT_REGLER_H_ */
