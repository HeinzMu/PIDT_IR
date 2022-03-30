#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#define CMD_MAX_ARGS        8

typedef enum eCmd{											// define the values the command returns
	CMD_NDEF	=	0,
	CMD_OK,
	CMD_BAD_CMD,
	CMD_TOO_MANY_ARGS,
	CMD_TOO_FEW_ARGS,
	CMD_WRONG_ARG,
	CMD_PORT_ERR
}enumCmd;

typedef struct{												// define the text that is shown depending of enumCmd
	const char *OK_TXT				= "- OK";
	const char *NDEF_TXT			= "- undefined case";
	const char *BAD_CMD_TXT			= "- CmdErr";
	const char *TOO_MANY_ARGS_TXT	= "- too many args";
	const char *TOO_FEW_ARGS_TXT	= "- too few args";
	const char *WRONG_ARG_TXT		= "- wrong arg";
	const char *PORT_ERR_TXT		= "- PortErr";
}txtCmd;

typedef int (*pfnCmdLine)(int argc, char *argv[]);

typedef struct{												// Structure for the command list table.
    const char *pcCmd;										// pointer to a string holding the  name of the command.
    pfnCmdLine pfnCmd;										// function pointer to the implementation of the command.
    const char *pcCmdDesc;									// pointer to a string holding the  description of the command.
	const char *pcHelp;										// pointer to a string to a help text for the command.
}tCmdLineEntry;

// Prototypes

void SerialClear(HardwareSerial);
int procRepCmd(int (*)(uint8_t), int, char **, int, unsigned long *, bool *);
int R_get_lvSollAcc(uint8_t);
int L_get_lvSollAcc(uint8_t);
int R_getIstSpeed(uint8_t);
int L_getIstSpeed(uint8_t);
int R_getSpeedStat(uint8_t);
int L_getSpeedStat(uint8_t);
int CmdLineProcess(char *);
int read_dig(int, char **);
int write_dig(int, char **);
int read_ana(int, char **);
int write_ana(int, char **);
int read_pwm(int, char **);
int write_pwm(int, char **);
int setReglerTypCmd(int, char **);
int getReglerTypCmd(int, char **);
int setReglerParamCmd(int, char **);
int getReglerParamCmd(int, char **);
int setSollSpeedCmd(int, char**);
int getSollSpeedCmd(int, char**);
int getRmpSollSpeedCmd(int, char**);
int getIstSpeedCmd(int, char**);
int setSollAccelCmd(int, char**);
int getSollAccelCmd(int, char**);
int getSpeedStatCmd(int, char**);
int getregVarsCmd(int, char**);
int getcontVarsCmd(int, char**);
int help(int, char **);

#endif // __CMDLINE_H__
