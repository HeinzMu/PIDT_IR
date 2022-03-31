#PIDT_IR Bibliothek

Mit dieser Bibliothek können Sie mit Gleichstrom betriebene Bürstenmotoren mit Encoder 
mit einem Arduino MEGA2560 regeln. Dis Softwareregelung wird über einen Timerinterrupt
zyklisch alle 5 Millisekunden aufgerufen. Um eine genaue Drehzahlregelung zu erreichen 
wird Timer3 für die Generierung eines Fast-PWM Signals für die Ausgänge 2 und 3 benutzt.

Diese Bibliothek ist nur für den Adruino MEGA2560 getestet.


Unterstützte Funktionen:

Regler initialisieren:
- void Init(tReglerTyp ReglerTyp, int iTacho_pin, int iTmon_pin, int iPWM_pin, void (*ISR_callback)(void), byte byMode = FALLING, int iOutLeft = 0, int iOutRight = 0, unsigned long ulRestartMotor = 200000L);
Regelparameter lesen:
- float getRegelParam(tReglerTyp RegelParam = REGLER_DEF);
Regelparameter setzen:
- void setRegelParam(tReglerTyp ReglerTyp, float fParam);
- void setRegelParam(tReglerTyp ReglerTyp, float fParam1, float fParam2);
Reglertyp setzen:
- tReglerTyp setRegler(tReglerTyp ReglerTyp);
Regler im Betrieb ohne Interrupt ausführen:
- unsigned long setRegler(tReglerTyp ReglerTyp, long lSoll, unsigned long ulStart);
Regler im Interruptbetrieb ausführen:
- void setReglerIR(void);
Soll-Geschwindigkeit setzen:
- void setSpeed(long lSoll);
Soll-Geschwindigkeit lesen:
- long getSollSpeed();
Ist-Geschwindigkeit lesen:
- float getIstSpeed();
Soll-Beschleunigung setzen:
- void setSollAccel(long lSollAccel);
Soll-Beschleunigung lesen:
- long getSollAccel(void);
Regel-Geschwindigkeit erreicht abfragen:
- bool getSpeedStat(void);
Regler-Variablen auslesen:
- regVars* get_regVars(void);
Rampen-Variablen auslesen:
- contVars* get_contVars(void);
Rampen-Geschwindigkeit lesen:
- long get_lvSollAcc();
Tacho lesen:
- void getTacho(void);


Das Beispiel 'Var_RPMs' ist für das Debugging mit dem avr_debugger vorbereitet.
Falls es bei Ihnen nicht funktioniert veresuchen Sie einen anderen Interrupt für den Debugger.

Das Beispiel 'cli4_PIDT_IR' ist ein Kommandozeilen Interface für den seriellen Monitor.

Diese Arbeit ist unter "Creative Commons Attribution-NonCommercial 4.0 International Lizenz" lizenziert.


