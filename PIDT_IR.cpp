#include <Arduino.h>
#include <PIDT_IR.h>
#include <myFunc.h>
#include <debug.h>
#include <myHWFunc.h>
#include <float.h>

byte PIDT_IR::bymyTaskID = 0;			// Instanzzähler

/*********************************************************************
 * @brief 					Initialisiere den Regler
 * 
 * @param ReglerTyp:		siehe tReglerTyp un PIDT_IR.h
 * @param iTacho_pin:		Input für Tachopin des Encoders
 * @param iTmon_pin:		Output für Monitorpin für TachiISR
 * @param iPWM_pin:			Output PWM für Motor
 * @param ISR_callback:		Zeiger zur TachoISR
 * @param byIntMode:		Mode zur TachoISR Aktivierung (RISING, FALLING, CHANGE)
 * @param iOutLeft:			Output-Pin Motor links drehen
 * @param iOutRight:		Output-Pin Motor rechts drehen
 * @param ulRestartMotor:	RestartZeit für den Motor bei Nichtanlauf
 * 
 ********************************************************************/
void PIDT_IR::Init(tReglerTyp ReglerTyp, int iTacho_pin, int iTmon_pin, int iPWM_pin, void (*ISR_callback)(void), byte byIntMode, int iOutLeft, int iOutRight, unsigned long ulRestartMotor){

	_rVars.bymyTaskID = bymyTaskID;
	if (_rVars.bymyTaskID > 2){
		SePLN(F("Es können max. 2 Instanzen von PIDT_IR initialisiert werden!"));
	}else{
		_rVars.selRegler = ReglerTyp;
		_iTacho_pin = iTacho_pin;
		_iTmon_pin = iTmon_pin;
		_iPWM_pin = iPWM_pin;
		_byIntMode = byIntMode;
		_rVars.ulRestartMotor = ulRestartMotor;
		_ulTacho = 0;
		_ulTachoCnt = 0;
		_ulLastChange = 0;
		_bTacho = false;
		_bTtoggle = false;
		_fAbw_ealt = 0;
		_ypt1_alt = 0;
#if (USER_MOTOR_TURN != BI)
		_bTurnBI = false;
#else
		_bTurnBI = true;
		_iOutLeft = iOutLeft;
		_iOutRight = iOutRight;
		pinMode(_iOutLeft, OUTPUT);
		pinMode(_iOutRight, OUTPUT);
#endif
		pinMode(_iTacho_pin, INPUT_PULLUP);
		if (_iTmon_pin != - 1) pinMode(_iTmon_pin, OUTPUT);
		char irqNum = digitalPinToInterrupt(_iTacho_pin);
		if (irqNum != NOT_AN_INTERRUPT) {
			attachInterrupt(irqNum, ISR_callback, _byIntMode);
		}else{
			SeP3LN(F("Can't attach pin: "), _iTacho_pin, F("to interrupt: "), irqNum);
		}
	}
}

/*********************************************************************
 * @brief 	ISR zum Einlesen des Encoder-Signals. \n
 * 			Berechnet die Zeit zwischen 2 Interrupts in µSec. \n 
 * 			Erhöht den Tacho Counter. \n 
 * 			Setzt flag _bTacho um anzuzeigen, dass Interrupt auftrat.
 * 
 ********************************************************************/
void PIDT_IR::getTacho(void){

	unsigned long ulCurrentTime = micros();
	_ulTacho  = ulCurrentTime - _ulLastChange;						// Duration = Time between edges
	_ulLastChange = ulCurrentTime;
	_ulTachoCnt++;													// Counter für Tacho
	if (_iTmon_pin > 0){
		digitalWrite(_iTmon_pin, _bTtoggle);						// Monitorausgang setzen
		_bTtoggle = !_bTtoggle;
	}
	_bTacho = true;
}

/*********************************************************************
 * @brief Berechnung den Betrag der Drehzahl in 1/min.
 * 
* @return float: für _bTacho = true  -> (USPMIN / USER_MOTOR_Inc_p_n) / _ulTacho in 1/min. \n 
 * 					 _bTacho = false -> fLastRes * (1 - exp(-1)) in 1/min \n 
 * 					 mit fLastRes = letzte Drehzahl.
 ********************************************************************/
float PIDT_IR::_readTacho(void){
	static float fLastRes = 0;
	float fRes;
	
	if (_bTacho){													// neuer Tachowert liegt vor
		_bTacho = false;
		fRes = (USPMIN / USER_MOTOR_Inc_p_n) / _ulTacho;
	}else{															// wenn kein neuer Wert vorliegt muss die Geschwindigkeit langsamer geworden sein
		fRes = fLastRes * (1 - exp(-1));
	}
	fRes = ((fRes < 1.2 * USER_MOTOR_N_MAX) ? fRes : _rVars.fSoll_w);
	fLastRes = fRes;
	return (fRes);
}

/*********************************************************************
 * @brief	Ruft den Regelalgorithmus auf und setzt den Regler. \n 
 * 			(Veraltet!!! Statt dessen sollte SetReglerIR() benutzt werden)
 * 
 * @param ReglerTyp:	siehe tReglerTyp un PIDT_IR.h
 * @param lSoll:		Solldrehzahl
 * @param ulStart:		Startzeit in µSec
 * 
 * @return unsigned long: Zeitdifferenz in µSec seit Startzeit
 * 
 ********************************************************************/
unsigned long PIDT_IR::setRegler(tReglerTyp ReglerTyp, long lSoll, unsigned long ulStart){

	_rVars.selRegler = ReglerTyp;
	_rVars.tStart = ulStart;
	_rVars.iDir = ((_rVars.lSoll >= 0) ? FORWARD : BACKWARD);
	if (lSoll == 0){
		_rVars.iPWM = 0;
		_rVars.fesum = 0;
		_fAbw_ealt = 0;
	}else{
		_rVars.fSoll_w = abs(lSoll);
		_rVars.fIst_x = _readTacho();								// istDrehzahl
		_calcRegler();												// neuen Sollwert berechnen
		_calcPWM();													// neuen Sollwert in PWM umrechnen
	}
	_SetDirAndPWM();												// Drehrichtung und PWM ausgeben
	_rVars.tEnde = micros();
	_rVars.ltDif = _rVars.tEnde - _rVars.tStart;
	return(_rVars.ltDif);
}

/*********************************************************************
 * @brief 	setReglerIR muss zyklisch alle x Millisekunden aufgerufen
 * 			werden. Die Eingestellte Zeit muss so gewählt werden, dass
 * 			der Regler schnell genug auf Lastwechsel oder neue Soll-
 * 			vorgaben reagieren kann. Für die Standard-Arduino DC-Motoren
 * 			habe ich die Zeit auf 5000 µSec gesetzt. \n 
 * 			Die Regelparameter Kp, Ki, Kd gelten IMMER nur für diese
 * 			eingestellte Zeit. \n 
 * 
 * 			!!! ACHTUNG !!!
 * 			Eine Rampe wird nur dann gefahren wenn: \n 
 * 			1.) Eine Beschleunigung für die Rampe eingestellt wurde. \n 
 * 			2.) Eine neue Soll-Drehzahl vorliegt.  \n 
 * 			!!! Nicht aber bei einem Lastwechsel. !!! \n
 * 
 ********************************************************************/
void PIDT_IR::setReglerIR(void){

	// die Regelparameter Kp, Ki, Kd gelten IMMER nur für die eingestellte TIMER1_TIME Zeit
	
	_rVars.ltDif = TIMER1_TIME;	

	if (_cVars.lSollAccel != 0){									// wenn Geschwindigkeitsrampe gefahren werden soll
		if (_rVars.lLastlSoll != _rVars.lSoll){						// wenn ein neuer Geschwindigkeits-Wert vorliegt
			_rVars.lLastlSoll = _rVars.lSoll;
																	// Beschleunigung (positiv / negativ) anhand des neuen Sollwerts berechnen
			_cVars.iAccDir = ((_rVars.lSoll >= _rVars.fIst_x * _rVars.iDir) ? ACC_ACC : ACC_DEC);
																	// Start-Drehzahl (+ / -) für die Beschleunigungrampe anhand der Drehrichtung festlegen 
			_cVars.lvStartAcc = ((_rVars.iDir == FORWARD) ? _rVars.fIst_x : -_rVars.fIst_x);
		}
	}
	
	_rVars.iDir = (_rVars.lSoll >= 0) ? FORWARD : BACKWARD;
	_rVars.fSoll_w = abs(_rVars.lSoll);
	_rVars.fIst_x = _readTacho();									// ist-Drehzahl

	_processRamp();													// Rampe behandeln
	_calcRegler();													// neuen Sollwert berechnen
	_calcPWM();														// neuen Sollwert in PWM umrechnen
	
	_SetDirAndPWM();												// Drehrichtung und PWM ausgeben
	
	_rVars.tEnde = micros();
}

/*********************************************************************
 * @brief 	Berechne in Abhängigkeit des Reglertyps den \n 
 * 			neuen normierten Sollwert
 * 
 ********************************************************************/
void PIDT_IR::_calcRegler(void){

	_rVars.fAbw_e = _rVars.fSoll_w - _rVars.fIst_x;					// Regelabweichung in 1/min
	_rVars.fesum += _rVars.fAbw_e;									// Summe aller Regelabweichungen in 1/min	berechnen
	//
	// die Summe auf einen max.Wert begrenzen (wie bei einem Kondensator)
	//
	if (abs(_rVars.fesum) > USER_MOTOR_ESUM_MAX) _rVars.fesum = sgn((long)_rVars.fesum) * USER_MOTOR_ESUM_MAX;
	//
	// Anzahl Regelungen pro Sekunde berechnen
	//
	if (_rVars.ltDif != 0)
		_rVars.fTa = (float)_rVars.ltDif / (float)1.0E6 ;			// in Sekunden umrechnen
	else
		_rVars.fTa = TA;
	//
	// Regelanteile berechnen
	//
	_rVars.yp = _rVars.fkp * _rVars.fAbw_e;							// Proportional-Anteil berechnen
	_rVars.yi = _rVars.fki * _rVars.fTa * _rVars.fesum;				// Integral-Anteil berechnen
																	// Differential-Anteil berechnen
	_rVars.yd = _rVars.fkd / _rVars.fTa * (_rVars.fAbw_e - _fAbw_ealt); 
	
	switch (_rVars.selRegler){
		case P_REGLER:
			_rVars.y   = _rVars.yp;
		break;
		case I_REGLER:
			_rVars.y = _rVars.yi;
		break;
		case D_REGLER:
			_rVars.y   = _rVars.yd;
		break;
		case PI_REGLER:
			_rVars.y    = _rVars.yp + _rVars.yi;
		break;
		case PD_REGLER:
			_rVars.y    = _rVars.yp + _rVars.yd;
		break;
		case ID_REGLER:
			_rVars.y    = _rVars.yi + _rVars.yd;
		break;
		case PID_REGLER:
			_rVars.y    = _rVars.yp + _rVars.yi + _rVars.yd;
		break;
		case PT1_REGLER:
			_rVars.ypt1 = (1 / ((_rVars.fTs * _rVars.fTa) + 1)) * (_rVars.fKs * _rVars.fAbw_e - _ypt1_alt) + _ypt1_alt;
			_rVars.y	= _rVars.ypt1;
			_ypt1_alt   = _rVars.y;
		break;
		default:
			LOGDEBUG(F("Kein Reglertyp ausgewählt"));
		break;
	}
	_fAbw_ealt = _rVars.fAbw_e;
}

/*********************************************************************
 * @brief 	Neuen Sollwert in PWM umrechnen \n
 * 			(TachoMax - TachoMin) entsprichch USER_MOTOR_PWM_MAX \n
 * 
 ********************************************************************/
void PIDT_IR::_calcPWM(){

	_rVars.fStell_y = USER_MOTOR_PWM_MAX / USER_MOTOR_N_MAX * _rVars.y;
	_rVars.iPWM = (int)_rVars.fStell_y;
	if (_rVars.fStell_y > USER_MOTOR_PWM_MAX)
		_rVars.iPWM = USER_MOTOR_PWM_MAX;
	if (_rVars.fStell_y < -USER_MOTOR_PWM_MAX)
		_rVars.iPWM = -USER_MOTOR_PWM_MAX;
	if ((_rVars.iPWM < USER_MOTOR_PWM_MIN && _rVars.fSoll_w >= 0) || (_rVars.iPWM > USER_MOTOR_PWM_MIN && _rVars.fSoll_w < 0))
		_rVars.iPWM = USER_MOTOR_PWM_MIN;
	//
	// !!!     Nur relevant im nicht Interrupt Betrieb !!!
	// wenn ltDif > 200 mSec ist, dann läuft der Motor nicht und es wird ein vmax Impuls ausgegeben
	//
	if (_rVars.ltDif > _rVars.ulRestartMotor){
		_rVars.iPWM = USER_MOTOR_PWM_MAX / 2;
		LOGERRORLN(F("Die max. Delayzeit ist überschritten.\nErhöhe entweder die Delayzeit in der Init-Funktion oder\nverkleinere die Delayzeiten in Programm"));
	}
}

/*********************************************************************
 * @brief 	Drehrichtung und Drehzahl setzen
 * 
 ********************************************************************/
void PIDT_IR::_SetDirAndPWM(){

	if (_bTurnBI){													// nur wenn beider Drehrichtungen zulässig sind
		if (_rVars.iDir == FORWARD){
			digitalWrite(_iOutLeft, HIGH);							//rechts drehen
			digitalWrite(_iOutRight, LOW);
		}else{
			digitalWrite(_iOutLeft, LOW);							//links drehen
			digitalWrite(_iOutRight, HIGH);
		}
	}
	my_analogWrite(_iPWM_pin, _rVars.iPWM);							// PWM ausgeben
}

/*********************************************************************
 * @brief 	Rampe abschalten wenn Drehzahl erreicht wurde
 * 
 ********************************************************************/
void PIDT_IR::_disableRamp(){

	_cVars.bAccActive = false;										// Rampe ist fertig
	_cVars.iAccDir = ACC_DIS;										// Rampenbeschleunigung abschalten
	_cVars.lvSollAcc = _rVars.lSoll;								// neue Solldrehzahl-Wert = Drehzahl Sollvorgabe setzen
	_cVars.iAccCnt = 0;												// Beschleunigungs-Counter rücksetzen
}

/*********************************************************************
 * @brief 	Nächste Solldrehzahl der Rampe berechnen
 * 			
 * 
 ********************************************************************/
void PIDT_IR::_processRamp(){

	if ((_cVars.iAccDir == ACC_DIS) && (_rVars.fSoll_w == 0)){
		_rVars.iPWM = 0;
		_rVars.fesum = 0;
		_fAbw_ealt = 0;
		_cVars.iAccCnt = 0;
		_cVars.bAccActive = false;									// Rampe ist fertig
	}else if (_cVars.iAccDir != ACC_DIS){
		_cVars.iAccCnt++;
		if (_cVars.iAccDir == ACC_ACC){
			_cVars.lvSollAcc = (long)(_cVars.iAccCnt * _cVars.lSollAccel * (float)TIMER1_TIME / 1E6 + _cVars.lvStartAcc);			// wenn die Beschleunigung zu gro� ist dann portionieren
			if (_cVars.lvSollAcc >= _rVars.lSoll){
				_cVars.iAccDir = ACC_DEC;
				_cVars.lvSollAcc = _rVars.lSoll;
				_cVars.iAccCnt = 0;
				_cVars.bAccActive = false;										// Rampe ist fertig
			}
		}else{
			_cVars.lvSollAcc = (long)(-_cVars.iAccCnt * _cVars.lSollAccel * (float)TIMER1_TIME / 1E6 + _cVars.lvStartAcc);			// wenn die Beschleunigung zu gro� ist dann portionieren
			if (_cVars.lvSollAcc <= _rVars.lSoll){
				_cVars.iAccDir = ACC_DIS;
				_cVars.lvSollAcc = _rVars.lSoll;
				_cVars.iAccCnt = 0;
				_cVars.bAccActive = false;										// Rampe ist fertig
			}
		}
		_rVars.iDir = sgn(_cVars.lvSollAcc);									// Drehrichtung setzen
		_rVars.fSoll_w = abs(_cVars.lvSollAcc);									// Regler Sollwertt setzen
	}
}

/*********************************************************************
 * @brief 	Setze die Regelparameter für die Regler
 * 
 * @param ReglerTyp:	 
 * @param fParam:		float-Wert für den Parameter
 * 
 ********************************************************************/
void PIDT_IR::setRegelParam(tReglerTyp ReglerTyp, float fParam){

	switch (ReglerTyp){
		case P_REGLER:
			_rVars.fkp = fParam;
		break;
		case I_REGLER:
			_rVars.fki = fParam;
		break;
		case D_REGLER:
			_rVars.fkd = fParam;
		break;
		case PT1_REGLER:
			_rVars.fKs = fParam;
		break;
		default:
			SeP1LN(F("Ungültiger Reglertyp "), ReglerTyp);
		break;
	}
}

/*********************************************************************
 * @brief 	Setze die Regelparameter für die Regler mit 2 Parameter
 * 
 * @param ReglerTyp:	 
 * @param fParam1:		float-Wert für den Parameter1. \n
 * @param fParam2:		float-Wert für den Parameter2.
 * 
 ********************************************************************/
void PIDT_IR::setRegelParam(tReglerTyp ReglerTyp, float fParam1, float fParam2){

	switch (ReglerTyp){
		case PT1_REGLER:
			_rVars.fKs = fParam1;
			_rVars.fTs = fParam2;
		break;
		default:
			SeP1LN(F("Ungültiger Reglertyp "), ReglerTyp);
		break;
	}
}

/*********************************************************************
 * @brief		Soll-Drehzahl setzen.
 * 				Wenn sich die lSoll und die aktive _rVars.lSoll Drehzahlen
 * 				unterscheiden und zusätzlich _bAccel gesetzt ist,
 * 				wird das Flag für Rampe-Aktiv gesetzt.
 * 
 * @param lSoll Drehzahl in 1/min
 * 
 ********************************************************************/
void PIDT_IR::setSpeed(long lSoll){
	if (_bAccel & (_rVars.lSoll != lSoll)) _cVars.bAccActive = true;
	_rVars.lSoll = (long)((abs(lSoll) <= USER_MOTOR_N_MAX) ? lSoll : sgn(lSoll) * USER_MOTOR_N_MAX);
}

/*********************************************************************
 * @brief	Soll-Drehzahl lesen
 * 
 * @return long Soll-Drehzahl in 1/min
 * 
 ********************************************************************/
long PIDT_IR::getSollSpeed(){
	return (_rVars.lSoll);
}

/*********************************************************************
 * @brief	Ist-Drehzahl lesen	(> 0 -> rechts drehend). \n 
 * 								(< 0 -> links  drehend). \n
 * 
 * @return	float Ist-Drehzahl in 1/min.
 * 
 ********************************************************************/
float PIDT_IR::getIstSpeed(){
	return(_rVars.fIst_x * _rVars.iDir);
}

/*********************************************************************
 * @brief		Soll-Beschleunigung setzen
 * 
 * @param lSollAccel Beschleunigung in 1/(min*Sec)
 * 
 ********************************************************************/
void PIDT_IR::setAccel(long lSollAccel){
	_cVars.lSollAccel = lSollAccel;
	_bAccel = (lSollAccel == 0L) ? false : true;
}

/*********************************************************************
 * @brief 	Status der Geschwindigkeit abfragen \n
 * 			Bei Rampenfahrt: -> bAccActive
 * 			Ohne Rampe: -> true wenn 0.98*IstSpeed > als lSoll \n 
 * 						-> false wenn < lSoll \n
 * 
 * @return	true  -> Rampenfahrt noch aktiv
 * 			false -> Rampenfahrt beendet
 * 
 ********************************************************************/
bool PIDT_IR::getSpeedStat(){

	bool bRet;
	if (!_bAccel){
		bRet = !((long)(abs(getIstSpeed() / 0.95)) >= abs(_rVars.lSoll));
	}else{
		bRet = _cVars.bAccActive;
	}
	return (bRet);
}

/*********************************************************************
 * @brief		Regelparameter lesen
 * 
 * @param 	ReglerTyp
 * 
 * @return	float Parameter des Reglertyps. \n
 * 
 ********************************************************************/
float PIDT_IR::getRegelParam(tReglerTyp ReglerTyp){

	float frParam;
	switch (ReglerTyp){
		case P_REGLER:
			frParam = _rVars.fkp;
		break;
		case I_REGLER:
			frParam = _rVars.fki;
		break;
		case D_REGLER:
			frParam = _rVars.fkd;
		break;
		case PT1_REGLER_S:
			frParam = _rVars.fKs;
		break;
		case PT1_REGLER_T:
			frParam = _rVars.fTs;
		break;
		case REGLER_DEF:
		default:
			frParam = FLT_MAX;
		break;
	}
	return (frParam);
}

/*********************************************************************
 * @brief Hole den Zeiger auf _rVars
 * 
 * @return regVars Zeiger zur _rVars Struktur
 * 
 ********************************************************************/
regVars* PIDT_IR::get_regVars(){
	return (&_rVars);
}

/*********************************************************************
 * @brief Hole den Zeiger auf _cVars
 * 
 * @return contVars Zeiger zur _cVars Struktur
 * 
 ********************************************************************/
contVars* PIDT_IR::get_contVars(){
	return (&_cVars);
}

/*********************************************************************
 * @brief Get the number of active tasks of PIDT_IR
 * 
 * @return Number of tasks
 * 
 ********************************************************************/
byte PIDT_IR::get_taskCnt(){
	return (_rVars.bymyTaskID);
}

/*********************************************************************
 * @brief Read the value of target acceleration for rpm
 * 
 * @return long value of target acceleration for rpm
 * 
 ********************************************************************/
long PIDT_IR::get_lvSollAcc(){
	return (_cVars.lvSollAcc);
}

/*********************************************************************
 * @brief Read the value of acceleration counter
 * 
 * @return int value of iAccCnt
 * 
 ********************************************************************/
int PIDT_IR::get_AccCnt(){
	return (_cVars.iAccCnt);
}

/*********************************************************************
 * @brief 	Read actual value of deviation of the controller. \n
 * 			(difference of _rVars.fSoll_w - _rVars.fIst_x; in 1/min.
 * 
 * @return float value of iAccCnt
 * 
 ********************************************************************/
float PIDT_IR::get_aktAbw(){
	return((int)(_rVars.fAbw_e));
}
