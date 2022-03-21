

#ifndef MOTOR_H_
#define MOTOR_H_

//! \brief USER MOTOR & ID SETTINGS
// **************************************************************************
#define MOTOR_Type_I									// Motor mit Inkrementalgeber
#define MOTOR_Type_S									// Motor mit Sinus-Geber
#define MOROR_Tyte_T									// Motor mit Tacho

//! \brief Define each motor with a unique name and ID number
// DC Mororen
#define ep512F_2					    101
#define ELEGO_GM					    102
#define MY_ELEGO_GM                     103

#define USER_MOTOR MY_ELEGO_GM

#if (USER_MOTOR == ep512F_2)							// Name must match the motor #define
#define USER_MOTOR_TYPE                 MOTOR_Type_T	// Tachomotor
#define USER_MOTOR_TURN					UNI				// nur eine Drehrichtung
#define USER_MOTOR_kp					(30)			// Regleranteil kp
#define USER_MOTOR_ki					(2.4024)		// KI Anteil I-Regler
#define USER_MOTOR_kd					(0.01)			// KD Anteil D-Regler
#define USER_MOTOR_Ks					(21.0)			// Verstärkungsfaktor[1/min] PT1 Glieds
#define USER_MOTOR_Ts					(0.001)			// Zeitkonstante[sec] des PT1 Glieds
#define USER_MOTOR_ESUM_MAX				100000			// maximale Integrationssumme
#define USER_MOTOR_selRegler			(PID_REGLER)	// PID-Regler ist standard
#define USER_MOTOR_Inc_p_n				(2)				// Inkremente pro Umdrehung

#elif (USER_MOTOR == ELEGO_GM)
#define USER_MOTOR_TYPE                 MOTOR_Type_I	// Incrementalgeber Motor
#define USER_MOTOR_TURN					BI				// cw und ccw Drehrichtung
#define USER_MOTOR_kp					(0.3497)		// Regleranteil kp
#define USER_MOTOR_ki					(3.05)			// KI Anteil I-Regler
#define USER_MOTOR_kd					(0.4)			// KD Anteil D-Regler
#define USER_MOTOR_Ks					(40.0)			// Verstärkungsfaktor[1/min] PT1 Glieds
#define USER_MOTOR_Ts					(0.001)			// Zeitkonstante[sec] des PT1 Glieds
#define USER_MOTOR_ESUM_MAX				100000			// maximale Integrationssumme
#define USER_MOTOR_selRegler			(PID_REGLER)	// PID-Regler ist standard
#define USER_MOTOR_N_MAX				(2800)			// in 1 / Minute
#define USER_MOTOR_PWM_MIN				(0.0)
#define USER_MOTOR_PWM_MAX				(1023.0)
#define USER_MOTOR_Inc_p_n				(20)			// Inkremente pro Umdrehung

#elif (USER_MOTOR == MY_ELEGO_GM)
#define USER_MOTOR_TYPE                 MOTOR_Type_I	// Incrementalgeber Motor
#define USER_MOTOR_TURN					BI				// cw und ccw Drehrichtung
#define USER_MOTOR_kp					(2)				// Regleranteil kp
#define USER_MOTOR_ki					(20)			// KI Anteil I-Regler
#define USER_MOTOR_kd					(0.0005)		// KD Anteil D-Regler
#define USER_MOTOR_Ks					(33.336)		// Verstärkungsfaktor[1/min] PT1 Glieds
#define USER_MOTOR_Ts					(0.026)			// Zeitkonstante[sec] des PT1 Glieds
#define USER_MOTOR_ESUM_MAX				100000			// maximale Integrationssumme
#define USER_MOTOR_selRegler			(PID_REGLER)	// PID-Regler ist standard
#define USER_MOTOR_N_MAX				(6750)			// in 1 / Minute
#define USER_MOTOR_PWM_MIN				(0.0)
#define USER_MOTOR_PWM_MAX				(1023.0)
#define USER_MOTOR_Inc_p_n				(30)			// Inkremente pro Umdrehung
#define USER_MOTOR_GETRIEBE				(48 / 1)		// Getriebe Übersetzung

#else
#error No motor type specified
#endif

#endif // endif MOTOR_H
