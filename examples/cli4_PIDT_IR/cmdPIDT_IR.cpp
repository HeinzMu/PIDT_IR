#include <Arduino.h>
#include <debug.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <PIDT_IR.h>
#include "HW_TRICYCLE_V2001.h"
#if SW_DEBUG
#include <avr8-stub.h>
#endif
#define USER_MOTOR MY_ELEGO_GM
#include <Motors.h>
#include <TimerOne.h>
#include <SoftWatchDog.h>
#include "cmdPIDT_IR.h"

PIDT_IR Regler_R, Regler_L;

tCmdLineEntry g_sCmdTable[] = {{"h", help, "help", "this help"},
							   {"rd", read_dig, "Digital read", "Portnummer 0..69 <r d>"},				// Digital lesen
							   {"wd", write_dig, "Digital write", "Portnummer 0..69 0/1"},				// Digital schreiben
							   {"ra", read_ana, "Analog read", "Portnummer 0..15 <r d>"},				// Analog lesen
							   {"wa", write_ana, "Analog write", "Portnummer 0..15 0..1023"},			// Analog schreiben
							   {"rp", read_pwm, "PWM read", "Portnummer 0..13"},						// PWM lesen
							   {"wp", write_pwm, "PWM write", "Portnummer 0..14 0..255"},				// PWM schreiben
							   {"srt", setReglerTypCmd, "Set ReglerTyp", "R/L P/I/D/S/T"},				// set Reglertyp
							   {"grt", getReglerTypCmd, "Get ReglerTyp", "R/L"},						// get Reglertyp
							   {"srp", setReglerParamCmd, "Set RegelParam", "R/L P/I/D/S/T -9.9E-32..+9.9E32"},		// set Regelparameter
							   {"grp", getReglerParamCmd, "Get RegelParam", "R/L P/I/D/S/T"},			// get Regelparameter
							   {"sss", setSollSpeedCmd, "Set SollSpeed", "R/L -5000..+5000"},			// set soll speed
							   {"gss", getSollSpeedCmd, "Get SollSpeed", "R/L"},						// set soll speed
							   {"grs", getRmpSollSpeedCmd, "Get RmpSollSpeed", "R/L <r d>"},			// get ramp soll speed
							   {"gis", getIstSpeedCmd, "Get IstSpeed", "R/L <r d>"},					// get ist speed
							   {"ssa", setSollAccelCmd, "Set SollAccel", "R/L 0..+20000"},				// set soll acceleration
							   {"gsa", getSollAccelCmd, "Get SollAccel", "R/L"},						// get soll acceleration
							   {"gst", getSpeedStatCmd, "Get SpeedStatus", "R/L <r d>"},				// get speed state
							   {"grv", getregVarsCmd, "Get regVars", "R/L"},							// get regulator values
							   {"gcv", getcontVarsCmd, "Get contVars", "R/L"}							// get controller values
							   };

void SerialClear(HardwareSerial mySer){
	while (mySer.available() > 0){
		mySer.read();
	}
}

int procRepCmd(int (*pfn)(uint8_t), int arg, char *av[], int iPort, unsigned long *lDelay, bool *bRep){

	WATCH W1;

	if ((arg > 2) & (arg <= 4)){
		if (*av[2] != 'r'){
			return (CMD_WRONG_ARG);
		}
		*bRep = true;
		if (arg == 4){
			*lDelay = atol(av[3]);
			if ((*lDelay < 0L) | (*lDelay > 60000L)){
				return (CMD_WRONG_ARG);
			}
			*lDelay *= 1000L;
		}
	}
	W1.watch(*lDelay);
   	SePLN(pfn(iPort));
	if (bRep){
		do{
			if (MY_SERIAL.available())
				*bRep = false;
			if (!W1.watch()){
				W1.watch(*lDelay); 
   				SePLN(pfn(iPort));
			}
		}while (*bRep);
		SerialClear(MY_SERIAL);
	}
	return (CMD_OK);
}

int R_get_lvSollAcc(uint8_t ui){
	return((int)Regler_R.get_lvSollAcc());
}

int L_get_lvSollAcc(uint8_t ui){
	return((int)Regler_L.get_lvSollAcc());
}

int R_getIstSpeed(uint8_t ui){
	return((int)Regler_R.getIstSpeed());
}

int L_getIstSpeed(uint8_t ui){
	return((int)Regler_L.getIstSpeed());
}

int R_getSpeedStat(uint8_t ui){
		return ((int)Regler_R.getSpeedStat());
}

int L_getSpeedStat(uint8_t ui){
		return ((int)Regler_L.getSpeedStat());
}

/*****************************************************************************
 * @brief           Easy to use command line interpreter
 * 
 * @param pcCmdLine space separeted command line with arguments (optopnal)
 * @return int      result of the command
 ****************************************************************************/
int CmdLineProcess(char *pcCmdLine){
	
    static char *argv[CMD_MAX_ARGS + 1];
    char *pcChar;
    int argc;
    int bFindArg = 1;
    tCmdLineEntry *pCmdEntry;

    argc = 0;
    pcChar = pcCmdLine;

    while(*pcChar){													// check if argument seperator
        if(*pcChar == ' '){
            *pcChar = 0;
            bFindArg = 1;											// set flag for argument exists
        }else{														// otherwise it must be part of an argument
            if(bFindArg){											// if argument exists
                if(argc < CMD_MAX_ARGS){							// check if arguments are lt max.arguments
                    argv[argc] = pcChar;							// read argument
                    argc++;
                    bFindArg = 0;
                }else{												// returm error
                    return (CMD_TOO_MANY_ARGS);
                }
            }
        }
        pcChar++;													// read next char
    }

    if(argc){														// check if one or more arguments are found
        //
        // Start at the beginning of the command table, to .
        //
        pCmdEntry = &g_sCmdTable[0];								// look for a matching command it the command table
        while(pCmdEntry->pcCmd){
            if(!strcmp(argv[0], pCmdEntry->pcCmd)){					// If this command entry command string matches argv[0],
				return (pCmdEntry->pfnCmd(argc, argv));				// call the function for this command and pass all the arguments.
            }
            pCmdEntry++;											// if not found advance to next entry
        }
    }
    return (CMD_BAD_CMD);											// reahing this point no matching command was found, so return an error
}

int generic(int arg, char *av[]){									// Template for a function
	return (arg);													// look at read_dig for an existing function
}

int read_dig(int arg, char *av[]){

	int iPort;
	unsigned long lDelay = 0;
	bool bRep = false;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}else{
		iPort = atoi(av[1]);
		if (iPort < 70){
			pinMode(iPort, INPUT);
			return (procRepCmd(digitalRead, arg, av, iPort, &lDelay, &bRep));
        }else{
            return (CMD_PORT_ERR);
		}

	}
}

int write_dig(int arg, char *av[]){
	if(arg != 3)
		return (CMD_TOO_FEW_ARGS);
	else{
		int iPort = atoi(av[1]);
		pinMode(iPort, OUTPUT);
		digitalWrite(iPort, atoi(av[2]));
	}
	return (CMD_OK);
}

int read_ana(int arg, char *av[]){

	int iPort;
	unsigned long lDelay = 0;
	bool bRep = false;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}else{
		iPort = atoi(av[1]);
		iPort = analogInputToDigitalPin(iPort);
		if (iPort != -1){
			return (procRepCmd(analogRead, arg, av, iPort, &lDelay, &bRep));
        }else{
            return (CMD_PORT_ERR);
		}
	}
}

int write_ana(int arg, char *av[]){

	int iPort;

	if(arg != 3){
		return (CMD_TOO_FEW_ARGS);
	}else{
		iPort = atoi(av[1]);
		analogWrite(iPort, atoi(av[2]));
	}
	return (CMD_OK);
}

int read_pwm(int arg, char *av[]){
	if (arg != 2)
		return (CMD_TOO_FEW_ARGS);
	else{
		int iPort = atoi(av[1]);
		pinMode(iPort,INPUT);
		SePLN(analogRead(iPort));
	}
	return (CMD_OK);
}

int write_pwm(int arg, char *av[]){
	if(arg != 3)
		return (CMD_TOO_FEW_ARGS);
	else{
		int iPort = atoi(av[1]);
		pinMode(iPort,OUTPUT);
		analogWrite(iPort, atoi(av[2]));
	}
	return (CMD_OK);
}

tReglerTyp getReglerTyp(char *av){

	tReglerTyp tR = REGLER_DEF;

	if (strcmp(av, "P") == 0){
		tR = P_REGLER;
	}else if (strcmp(av, "I") == 0){
		tR = I_REGLER;
	}else if (strcmp(av, "D") == 0){
		tR = D_REGLER;
	}else if (strcmp(av, "PI") == 0){
		tR = PI_REGLER;
	}else if (strcmp(av, "PD") == 0){
		tR = PD_REGLER;
	}else if (strcmp(av, "ID") == 0){
		tR = ID_REGLER;
	}else if (strcmp(av, "PID") == 0){
		tR = PID_REGLER;
	}else if (strcmp(av, "PT1") == 0){
		tR = PT1_REGLER;
	}
	return (tR);
}

int setReglerTypCmd(int arg, char *av[]){

	if (arg < 3){
		return (CMD_TOO_FEW_ARGS);
	}

	tReglerTyp tR = REGLER_DEF;

	if ((*av[1] == 'R') | (*av[1] == 'L')){
		tR = getReglerTyp(av[2]);
		if (tR == REGLER_DEF){
			return (CMD_WRONG_ARG);
		}
		if (*av[1] == 'R'){
			Regler_R.setRegler(tR);
		}else if (*av[1] == 'L'){
			Regler_L.setRegler(tR);
		}
		return (CMD_OK);
	}else{
		return (CMD_WRONG_ARG);
	}
}

int getReglerTypCmd(int arg, char *av[]){

	tReglerTyp tR;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}
	if (*av[1] == 'R'){
		tR = (tReglerTyp)Regler_R.getRegelParam();
	}else if (*av[1] == 'L'){
		tR = (tReglerTyp)Regler_L.getRegelParam();
	}else{
		return (CMD_WRONG_ARG);
	}
	SeP1LN(F("Reglertyp: "), (unsigned int)tR);
	return (CMD_OK);
}

int setReglerParamCmd(int arg, char *av[]){

	if (arg < 4){
		return (CMD_TOO_FEW_ARGS);
	}

	float fParam = (float)atof(av[3]);
	tReglerTyp tR;

	if ((*av[1] != 'R') | (*av[1] != 'L')){
		tR = getReglerTyp(av[2]);
		switch (tR){
			case P_REGLER:
			case I_REGLER:
			case D_REGLER:
			case PT1_REGLER_S:
			case PT1_REGLER_T:
				if (*av[1] == 'R'){
					Regler_R.setRegelParam(tR, fParam);
				}else{
					Regler_L.setRegelParam(tR, fParam);
				}
				return (CMD_OK);
			break;
			default:
				return (CMD_WRONG_ARG);
			break;
		}
	}else{
		return (CMD_WRONG_ARG);
	}
}

int getReglerParamCmd(int arg, char *av[]){

	int rParam;
	tReglerTyp tR;

	if (arg < 3){
		return (CMD_TOO_FEW_ARGS);
	}
	
	if (arg == 3){
		if ((*av[1] != 'R') | (*av[1] != 'L')){
			tR = getReglerTyp(av[2]);
			switch (tR){
				case P_REGLER:
					SeP1LN(F("P: "), Regler_R.getRegelParam(P_REGLER));
				break;
				case I_REGLER:
					SeP1LN(F("I: "), Regler_R.getRegelParam(I_REGLER));
				break;
				case D_REGLER:
					SeP1LN(F("D: "), Regler_R.getRegelParam(D_REGLER));
				break;
				case PI_REGLER:
					SeP1LN(F("P: "), Regler_R.getRegelParam(P_REGLER));
					SeP1LN(F("I: "), Regler_R.getRegelParam(I_REGLER));
				break;
				case PD_REGLER:
					SeP1LN(F("P: "), Regler_R.getRegelParam(P_REGLER));
					SeP1LN(F("D: "), Regler_R.getRegelParam(D_REGLER));
				break;
				case PID_REGLER:
					SeP1LN(F("P: "), Regler_R.getRegelParam(P_REGLER));
					SeP1LN(F("I: "), Regler_R.getRegelParam(I_REGLER));
					SeP1LN(F("D: "), Regler_R.getRegelParam(D_REGLER));
				break;
				case PT1_REGLER_S:
					SeP1LN(F("PT1_S: "), Regler_R.getRegelParam(PT1_REGLER_S));
				break;
				case PT1_REGLER_T:
					SeP1LN(F("PT1_T: "), Regler_R.getRegelParam(PT1_REGLER_T));
				break;
				case PT1_REGLER:
					SeP1LN(F("PT1_S: "), Regler_R.getRegelParam(PT1_REGLER_S));
					SeP1LN(F("PT1_T: "), Regler_R.getRegelParam(PT1_REGLER_T));
				break;
				default:
					return(CMD_WRONG_ARG);
				break;
			}
			return (CMD_OK);
		}else{
			return (CMD_WRONG_ARG);
		}
	}else if (arg == 2){
		rParam = (int)Regler_R.getRegelParam();
		SeP1LN(F("Reglertyp: "), rParam);
		return (CMD_OK);
	}

	return (CMD_TOO_FEW_ARGS);
}

int setSollSpeedCmd(int arg, char *av[]){

	if (arg < 3){
		return (CMD_TOO_FEW_ARGS);
	}

	long lSpeed = atol(av[2]);

	if ((*av[1] == 'R') | (*av[1] == 'L') | ((lSpeed >= -5000) & (lSpeed <= 5000))){
		if (*av[1] == 'R'){
			Regler_R.setSpeed(lSpeed);
		}else if (*av[1] == 'L'){
			Regler_L.setSpeed(lSpeed);
		}
		return (CMD_OK);	
	}else{
		return (CMD_WRONG_ARG);
	} 
}

int getSollSpeedCmd(int arg, char *av[]){

	long lS;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}
	if (*av[1] == 'R'){
		SeP(F("SollSpeed R: "));
		lS = Regler_R.getSollSpeed();
	}else if (*av[1] == 'L'){
		SeP(F("SollSpeed L: "));
		lS = Regler_L.getSollSpeed();
	}else{
		return (CMD_WRONG_ARG);
	}
	SePLN(lS);
	return (CMD_OK);
}

int getRmpSollSpeedCmd(int arg, char *av[]){

	unsigned long lDelay = 0;
	bool bRep = false;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}
	if (*av[1] == 'R'){
		SeP(F("Soll Ramp Speed R: "));
		return (procRepCmd(R_get_lvSollAcc, arg, av, 0, &lDelay, &bRep));
	}else if (*av[1] == 'L'){
		SeP(F("Soll Ramp Speed L: "));
		return (procRepCmd(L_get_lvSollAcc, arg, av, 0, &lDelay, &bRep));
	}else{
		return (CMD_WRONG_ARG);
	}
}

int getIstSpeedCmd(int arg, char *av[]){

	unsigned long lDelay = 0;
	bool bRep = false;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}
	if (*av[1] == 'R'){
		SeP(F("IstSpeed R: "));
		return (procRepCmd(R_getIstSpeed, arg, av, 0, &lDelay, &bRep));
	}else if (*av[1] == 'L'){
		SeP(F("IstSpeed L: "));
		return (procRepCmd(L_getIstSpeed, arg, av, 0, &lDelay, &bRep));
	}else{
		return (CMD_WRONG_ARG);
	}
}

int setSollAccelCmd(int arg, char *av[]){

	if (arg < 3){
		return (CMD_TOO_FEW_ARGS);
	}

	long lAccel = atol(av[2]);

	if ((*av[1] == 'R') | (*av[1] == 'L') | ((lAccel >= 0) & (lAccel <= 20000))){
		if (*av[1] == 'R'){
			Regler_R.setSollAccel(lAccel);
		}else if (*av[1] == 'L'){
			Regler_L.setSollAccel(lAccel);
		}
		return (CMD_OK);
	}else{
		return (CMD_WRONG_ARG);
	}
}

int getSollAccelCmd(int arg, char *av[]){

	long lS;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}
	if (*av[1] == 'R'){
		SeP(F("IstSpeed R: "));
		lS = Regler_R.getSollAccel();
	}else if (*av[1] == 'L'){
		SeP(F("IstSpeed L: "));
		lS = Regler_L.getIstSpeed();
	}else{
		return (CMD_WRONG_ARG);
	}
	SePLN(lS);
	return (CMD_OK);
}

int getSpeedStatCmd(int arg, char *av[]){

	bool bRep;
	unsigned long lDelay;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}
	if (*av[1] == 'R'){
		SeP(F("Speed Status R: "));
		return (procRepCmd(R_getSpeedStat, arg, av, 0, &lDelay, &bRep));
	}else if (*av[1] == 'L'){
		SeP(F("Speed Status L: "));
		return (procRepCmd(R_getSpeedStat, arg, av, 0, &lDelay, &bRep));
	}else{
		return (CMD_WRONG_ARG);
	}
}

int getregVarsCmd(int arg, char *av[]){

	regVars *rV;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}
	if (*av[1] == 'R'){
		SePLN(F("Regulator vars R: "));
		rV = Regler_R.get_regVars();
	}else if (*av[1] == 'L'){
		SePLN(F("Regulator vars L: "));
		rV = Regler_L.get_regVars();
	}else{
		return (CMD_WRONG_ARG);
	}
	SeP1LN(F("lSoll: "), rV->lSoll);
	SeP1LN(F("lLastlSoll: "), rV->lLastlSoll);
	SeP1LN(F("fIst_x: "), rV->fIst_x);
	SeP1LN(F("fSoll_w: "), rV->fSoll_w);
	SeP1LN(F("fAbw_e: "), rV->fAbw_e);
	SeP1LN(F("fesum: "), rV->fesum);
	SeP1LN(F("fTa: "), rV->fTa);
	SeP1LN(F("fStell_y: "), rV->fStell_y);
	SeP1LN(F("yp: "), rV->yp);
	SeP1LN(F("yi: "), rV->yi);
	SeP1LN(F("yd: "), rV->yd);
	SeP1LN(F("ypt1: "), rV->ypt1);
	SeP1LN(F("y: "), rV->y);
	SeP1LN(F("iPWM: "), rV->iPWM);
	SeP1LN(F("fkp: "), rV->fkp);
	SeP1LN(F("fki: "), rV->fki);
	SeP1LN(F("fkd: "), rV->fkd);
	SeP1LN(F("fKs: "), rV->fKs);
	SeP1LN(F("fTs: "), rV->fTs);
	SeP1LN(F("selRegler: "), (int)rV->selRegler);
	SeP1LN(F("tStart: "), rV->tStart);
	SeP1LN(F("tEnde: "), rV->tEnde);
	SeP1LN(F("ltDif: "), rV->ltDif);
	SeP1LN(F("ulRestartMotor: "), rV->ulRestartMotor);
	SeP1LN(F("iDir: "), rV->iDir);
	SeP1LN(F("bymyTaskID: "), rV->bymyTaskID);
	return (CMD_OK);
}

int getcontVarsCmd(int arg, char *av[]){

	contVars *rV;

	if (arg < 2){
		return (CMD_TOO_FEW_ARGS);
	}
	if (*av[1] == 'R'){
		SePLN(F("Regulator vars R: "));
		rV = Regler_R.get_contVars();
	}else if (*av[1] == 'L'){
		SePLN(F("Regulator vars L: "));
		rV = Regler_L.get_contVars();
	}else{
		return (CMD_WRONG_ARG);
	}
	SeP1LN(F("lSollAccel: "), rV->lSollAccel);
	SeP1LN(F("lvStartAcc: "), rV->lvStartAcc);
	SeP1LN(F("lvSollAcc: "), rV->lvSollAcc);
	SeP1LN(F("iAccCnt: "), rV->iAccCnt);
	SeP1LN(F("iAccDir: "), rV->iAccDir);
	SeP1LN(F("bAccActive: "), rV->bAccActive);
	return (CMD_OK);
}

int help(int arg, char *av[]){

	char c[64];
    size_t ts = sizeof(g_sCmdTable) / sizeof(g_sCmdTable[0]);

	SePLN("Supported commands:");
	for (size_t is = 0; is < ts; is++){
		sprintf(c, "%-20s: %-3s %-s", g_sCmdTable[is].pcCmdDesc, g_sCmdTable[is].pcCmd, g_sCmdTable[is].pcHelp);
		SePLN(c);
	}
	SePLN();
	SePLN(F("The optional parameter have the meaning:"));
	SePLN(F("r:	read repeated until key pressed"));
	SePLN(F("d: set delay 1.. im mSec for repeated read"));
	SePLN();
	return (CMD_OK);
}
