#define AKKU_STATUS   A0				// Analoger Eingang für Akku
#define ANA_IN1		  A1				// nc
#define ANA_IN2		  A2				// nc
#define ANA_IN3		  A3				// nc
#define ANA_IN4		  A4				// nc
#define ANA_IN5		  A5				// nc
#define ANA_IN6		  A6				// nc
#define ANA_IN7		  A7				// nc

#define ANA_IN8		  A8				// nc
#define ANA_IN9		  A9				// nc
#define ANA_IN10	  A10				// nc
#define ANA_IN11	  A11				// nc
#define ANA_IN12	  A12				// nc
#define ANA_IN13	  A13				// nc
#define ANA_IN14	  A14				// nc
#define ANA_IN15	  A15				// nc

#define Tx0			  0					// RxD0 belegt durch Serial
#define Rx0			  1					// TxD0 belegt durch Serial
#define PWM_OUTR      2				 	// PWM_Out Motor rechts
#define PWM_OUTL      3				 	// PWM_Out Motor links
#define IO4			  4					// nc
#define IO5			  5					// nc
#define IO6			  6					// nc
#define IO7			  7					// nc

#define IO8			  8					// nc
#define IO9			  9					// nc
#define IO10		  10				// nc
#define IO11		  11				// nc
#define SF_IN 		  12			 	// Special Function Input
#define POWER_DOWN    13                // Power ausschalten

#define Tx3			  14				// nc (TxD3) 
#define Rx3			  15				// nc (RxD3)
#define Tx2			  16				// TxD für ESP-01
#define Rx2			  17				// RxD für ESP-01
#define INC_R         18				// Encoder Motor rechts
#define INC_L         19				// Encoder Motor links
#define SDA           20			    // I2C Kommunikation
#define SCL           21			    // I2C Kommunikation

#define LEFT_MR       22				// (PA0) Links drehen Motor rechts
#define RIGHT_MR      23				// (PA1) Rechts drehen Motor rechts
#define LEFT_ML       24				// (PA2) Links drehen Motor links
#define RIGHT_ML      25				// (PA3) Rechts drehen Motor links
#define PA4_          26				// nc (PA4)
#define PA5_          27				// nc (PA5)
#define INC_R_MON     28				// (PA6) Monitor Ausgang Tacho rechts
#define INC_L_MON     29				// (PA7) Monitor Ausgang Tacho links

#define PC7_          30				// nc (PC7)
#define PC6_          31				// nc (PC6)
#define PC5_          32				// nc (PC5)
#define O_XSHUT_5     33			    // (PC4) Enable distance sensors 5
#define O_XSHUT_4     34			    // (PC3) Enable distance sensors 4
#define O_XSHUT_3     35				// (PC2) Enable distance sensors 3
#define O_XSHUT_2     36				// (PC1) Enable distance sensors 2
#define O_XSHUT_1     37				// (PC0) Enable distance sensors 1

#define LED_AJ        38				// LED Disanzsensoren ok
#define LED_WLAN      39				// LED WLAN aktiv
#define LED_NVR       40				// LED Kolission
#define LED_LOW_BAT   41				// LED Accu low
#define PL7_          42				// nc
#define PL6_          43                // nc
#define PL5_          44                // nc
#define I_DIST5_GPIO1 45                // GPIO1 of distance sensors 5 if soldering jumper closed

#define I_DIST4_GPIO1 46                // GPIO1 of distance sensors 4 if soldering jumper closed
#define I_DIST3_GPIO1 47                // GPIO1 of distance sensors 3 if soldering jumper closed
#define I_DIST2_GPIO1 48                // GPIO1 of distance sensors 2 if soldering jumper closed
#define I_DIST1_GPIO1 49                // GPIO1 of distance sensors 1 if soldering jumper closed
#define MISO		  50				// SPI Miso
#define MOSI		  51				// SPI Mosi
#define SCK			  52				// SPI Clock
#define SS			  53				// SPI Chip Select

