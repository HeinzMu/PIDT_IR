#PIDT_IR Library#

With this library you can control brushed dc motors having an encoder attached with the Arduino Mega2560.
The regulator works with a timer interrupt and is called every 5 milli Seconds. To get a accurate RPM result Timer3
is used to generate a fast PWM signal at output 2 and 3.

The library is NOT tested for any other board than Arduino Mega2560.

Provided functions:

- void Init(tReglerTyp ReglerTyp, int iTacho_pin, int iTmon_pin, int iPWM_pin, void (*ISR_callback)(void), byte byMode = FALLING, int iOutLeft = 0, int iOutRight = 0, unsigned long ulRestartMotor = 200000L);
- float getRegelParam(tReglerTyp RegelParam = REGLER_DEF);
- void setRegelParam(tReglerTyp ReglerTyp, float fParam);
- void setRegelParam(tReglerTyp ReglerTyp, float fParam1, float fParam2);
- tReglerTyp setRegler(tReglerTyp ReglerTyp);
- unsigned long setRegler(tReglerTyp ReglerTyp, long lSoll, unsigned long ulStart);
- void setReglerIR(void);
- void setSpeed(long lSoll);
- long getSollSpeed();
- float getIstSpeed();
- void setSollAccel(long lSollAccel);
- long getSollAccel(void);
- bool getSpeedStat(void);
- regVars* get_regVars(void);
- contVars* get_contVars(void);
- long get_lvSollAcc();
- void getTacho(void);



The example 'Var_RPMs' is prepared to debug with the avr_debugger.
If it doesn't work try another interrupt for the degugger.

The example 'cli4_PIDT_IR' is a command line interface for use with the serial monitor.

This work is licensed under a Creative Commons Attribution-NonCommercial 4.0 International License.

