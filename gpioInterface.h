#include <stdlib.h>
#include <stdio.h>

#define RASPBERRYPI	0
#define ARIETTAG25	1
#define CUSTOM		2
//============================================================================
// Description : SidBerry wrapper to GPIO ports - Header File
// Author      : Alessio Lombardo
//============================================================================

#if BOARD == RASPBERRYPI				//Raspberry Pi

	#include <wiringPi.h>
	
	#define CS		3
	#define A0		8
	#define A1		9
	#define A2		7
	#define A3		0
	#define A4		2
	#define D0		15
	#define D1		16
	#define D2		1
	#define D3		4
	#define D4		5
	#define D5		6
	#define D6		10
	#define D7		11
	
#elif BOARD == ARIETTAG25				//Acme Systems Arietta G25

	#include <wiringSam.h>
	
	#define CS		65
	#define A0		6
	#define A1		92
	#define A2		68
	#define A3		67
	#define A4		66
	#define D0		8
	#define D1		0
	#define D2		29
	#define D3		28
	#define D4		27
	#define D5		26
	#define D6		25
	#define D7		24

#elif BOARD == CUSTOM					//Custom Board
	
	//1) GPIO library (if needed)
	//2) I/O Defines (if needed): INPUT mode, OUTPUT mode, HIGH level, LOW level
	//3) Pinout Defines (mandatory): CS (Chip Select), A0-A4 (Address Bus), D0-D7 (Data Bus)
	
#endif

void gpioWrite(int pin, int level);
int gpioSetup();
void gpioMode(int pin, int mode);