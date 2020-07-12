//============================================================================
// Description : SidBerry wrapper to GPIO ports
// Author      : Alessio Lombardo
//============================================================================

#include "gpioInterface.h"

#if BOARD == RASPBERRYPI				//Raspberry Pi (GPIO library: <wiringPi.h>)

void gpioWrite(int pin, int level){
	digitalWrite(pin, level);
}

int gpioSetup(){
	wiringPiSetup();
}

void gpioMode(int pin, int mode){
	pinMode(pin, mode);
}
	
#elif BOARD == ARIETTAG25				//Acme Systems Arietta G25 (GPIO library: <wiringSam.h>)

void gpioWrite(int pin, int level){
	if (pin>=0 && pin<32) digitalWrite(WSAM_PIO_A, pin, level);
	else if (pin>=32 && pin<64) digitalWrite(WSAM_PIO_B, pin-32, level);
	else if (pin>=64 && pin<96) digitalWrite(WSAM_PIO_C, pin-64, level);
}

int gpioSetup(){
	wiringSamSetup();
}

void gpioMode(int pin, int mode){
	FILE *fp=fopen("/sys/class/gpio/export","w");
	fprintf(fp,"%d",pin);
	fclose(fp);
	if (pin>=0 && pin<32) pinMode(WSAM_PIO_A, pin, mode);
	else if (pin>=32 && pin<64) pinMode(WSAM_PIO_B, pin-32, mode);
	else if (pin>=64 && pin<96) pinMode(WSAM_PIO_C, pin-64, mode);
}

#elif BOARD == CUSTOM					//Custom Board

/*
 *	Function to WRITE to a GPIO port. Map this function to the equivalent GPIO library function or write it.
 */
void gpioWrite(int pin, int level){
	
}

/*
 *	Function to SETUP the GPIO ports. Map this function to the equivalent GPIO library function or write it.
 */
int gpioSetup(){
}

/*
 *	Function to set the MODE of a GPIO port. Map this function to the equivalent GPIO library function or write it.
 */
void gpioMode(int pin, int mode){
}
	
#endif