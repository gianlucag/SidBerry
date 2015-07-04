//============================================================================
// Description : SidBerry player & driver 
// Author      : Gianluca Ghettini
//============================================================================

#include <iostream>
using namespace std;
#include <stdlib.h>
#include "mos6502.h"
#include "SidFile.h"
#include <wiringPi.h>
#include <sys/time.h>

uint8_t memory[65535]; // 64K ram
static uint8_t kk = 0;
static FILE* tty;

void TestWrite(uint16_t addr, uint8_t byte)
{	
		printf("\na: %02X, b: %02X", addr, byte);		
		
		uint16_t phyaddr = addr;
		
		// set address to the bus
		if(phyaddr & 0x01) digitalWrite(8, HIGH);
		else digitalWrite(8, LOW);
		if(phyaddr & 0x02) digitalWrite(9, HIGH);
		else digitalWrite(9, LOW);
		if(phyaddr & 0x04) digitalWrite(7, HIGH);
		else digitalWrite(7, LOW);
		if(phyaddr & 0x08) digitalWrite(0, HIGH);
		else digitalWrite(0, LOW);
		if(phyaddr & 0x10) digitalWrite(2, HIGH);
		else digitalWrite(2, LOW);
		
		// set data to the bus
		if(byte & 0x01) digitalWrite(15, HIGH);
		else digitalWrite(15, LOW);
		if(byte & 0x02) digitalWrite(16, HIGH);
		else digitalWrite(16, LOW);
		if(byte & 0x04) digitalWrite(1, HIGH);
		else digitalWrite(1, LOW);
		if(byte & 0x08) digitalWrite(4, HIGH);
		else digitalWrite(4, LOW);
		if(byte & 0x10) digitalWrite(5, HIGH);
		else digitalWrite(5, LOW);			
		if(byte & 0x20) digitalWrite(6, HIGH);
		else digitalWrite(6, LOW);
		if(byte & 0x40) digitalWrite(10, HIGH);
		else digitalWrite(10, LOW);
		if(byte & 0x80) digitalWrite(11, HIGH);
		else digitalWrite(11, LOW);		
		
		// assert cs line
		digitalWrite(3, LOW);
		usleep(10);		
		digitalWrite(3, HIGH);	

	printf("\nOK\n");

	return;
}

void MemWrite(uint16_t addr, uint8_t byte)
{	
	if(addr >= 0xD400 && addr <= 0xD7FF) // address decoding login
	{
		// access to SID chip
		
		memory[addr] = byte;
		
		if(!((kk++)&3))
		{
			printf("\n");	
			printf("%02X%02X ", memory[0xD400], memory[0xD401]);
			printf("%02X%02X ", memory[0xD402], memory[0xD403]);
			printf("%02X %02X %02X ", memory[0xD404], memory[0xD405], memory[0xD406]);
			printf("%02X%02X ", memory[0xD407], memory[0xD408]);
			printf("%02X%02X ", memory[0xD409], memory[0xD40A]);
			printf("%02X %02X %02X ", memory[0xD40B], memory[0xD40C], memory[0xD40D]);
			printf("%02X%02X ", memory[0xD40E], memory[0xD40F]);
			printf("%02X%02X ", memory[0xD410], memory[0xD411]);			
			printf("%02X %02X %02X ", memory[0xD412], memory[0xD413], memory[0xD414]);	
			fflush(stdout);
		}
		
		uint16_t phyaddr = addr & 0x1f;
		
		// set address to the bus
		if(phyaddr & 0x01) digitalWrite(8, HIGH);
		else digitalWrite(8, LOW);
		if(phyaddr & 0x02) digitalWrite(9, HIGH);
		else digitalWrite(9, LOW);
		if(phyaddr & 0x04) digitalWrite(7, HIGH);
		else digitalWrite(7, LOW);
		if(phyaddr & 0x08) digitalWrite(0, HIGH);
		else digitalWrite(0, LOW);
		if(phyaddr & 0x10) digitalWrite(2, HIGH);
		else digitalWrite(2, LOW);
		
		// set data to the bus
		if(byte & 0x01) digitalWrite(15, HIGH);
		else digitalWrite(15, LOW);
		if(byte & 0x02) digitalWrite(16, HIGH);
		else digitalWrite(16, LOW);
		if(byte & 0x04) digitalWrite(1, HIGH);
		else digitalWrite(1, LOW);
		if(byte & 0x08) digitalWrite(4, HIGH);
		else digitalWrite(4, LOW);
		if(byte & 0x10) digitalWrite(5, HIGH);
		else digitalWrite(5, LOW);			
		if(byte & 0x20) digitalWrite(6, HIGH);
		else digitalWrite(6, LOW);
		if(byte & 0x40) digitalWrite(10, HIGH);
		else digitalWrite(10, LOW);
		if(byte & 0x80) digitalWrite(11, HIGH);
		else digitalWrite(11, LOW);		
		
		// assert CS line (strobe)
		digitalWrite(3, LOW);
		usleep(1);		
		digitalWrite(3, HIGH);	
	}
	else
	{
		// access to memory
		memory[addr] = byte;
	}
	return;
}

uint8_t MemRead(uint16_t addr)
{
	if(addr >= 0xD400 && addr <=0xD7FF) // address decoding logic
	{
		// access to SID chip
		if((addr & 0x00FF) == 0x001B)
		{
			// emulate read access to OSC3/Random register, returna a random value
			printf("\mread! 1b");
			return rand();
		}
		if((addr & 0x00FF) == 0x001C)
		{
			// emulate access to envelope ENV3 register, return a random value
			printf("\nread! 1c");
			return rand();
		}
    }
	return memory[addr];
}

void TestLoop()
{
	int addr;
	int byte;
	printf("\n\n");

	while(1)
	{
		printf("\naddr:");
		scanf("%d", &addr);
		printf("\nbyte:");
		scanf("%d", &byte);
		TestWrite(addr, byte); 
	}	

}

int main(int argc, char *argv[])
{
	SidFile sid;
	
	int res = sid.Parse(argv[1]);
	
	if(res == SIDFILE_ERROR_FILENOTFOUND)
	{
		cout << endl << "error loading sid file! not found";
		return 0;
	}
	
	if(res == SIDFILE_ERROR_MALFORMED)
	{
		cout << endl << "error loading sid file! malformed";
		return 0;
	}
	
	cout << endl << sid.GetModuleName();
	cout << endl << sid.GetAuthorName();
	cout << endl << sid.GetCopyrightInfo();
	
	cout << endl << sid.GetLoadAddress();
	cout << endl << sid.GetInitAddress();
	cout << endl << sid.GetPlayAddress();
	
	for(unsigned int i = 0; i < 65536; i++)
	{
		memory[i] = 0x00; // fill with NOPs
	}
	
	uint16_t load = sid.GetLoadAddress();
	uint16_t len = sid.GetDataLength();
	uint8_t *buffer = sid.GetDataPtr();
	for(unsigned int i = 0; i <  len; i++)
	{
		memory[i + load] = buffer[i];
	}
	
	uint16_t play = sid.GetPlayAddress();
	uint16_t init = sid.GetInitAddress();
	
	
	// install reset vector for microplayer (0x0000)
	memory[0xFFFD] = 0x00;
	memory[0xFFFC] = 0x00;
	
	// install IRQ vector for play routine launcher (0x0013)
	memory[0xFFFF] = 0x00;
	memory[0xFFFE] = 0x13;
	

	// install the micro player, 6502 assembly code
	 
	memory[0x0000] = 0xA9; // A = 0, load A with the song number
	memory[0x0001] = atoi(argv[2]);
	
	memory[0x0002] = 0x20; // jump sub to INIT routine
	memory[0x0003] = init & 0xFF; // lo addr
	memory[0x0004] = (init >> 8) & 0xFF; // hi addr
	
	memory[0x0005] = 0x58; // enable interrupt
	memory[0x0006] = 0xEA; // nop
	memory[0x0007] = 0x4C; // jump to 0x0006
	memory[0x0008] = 0x06;
	memory[0x0009] = 0x00;
	
	memory[0x0013] = 0xEA; // nop  //0xA9; // A = 1
	memory[0x0014] = 0xEA; // nop //0x01;	
	memory[0x0015] = 0xEA; // 0x78 CLI
	memory[0x0016] = 0x20; // jump sub to play routine
	memory[0x0017] = play & 0xFF;
	memory[0x0018] = (play >> 8) & 0xFF;
	memory[0x0019] = 0xEA; // 0x58 SEI	
	memory[0x001A] = 0x40; // RTI: return from interrupt
	
	// setup wiringPi, configure GPIOs
	wiringPiSetup();
	
	pinMode(15, OUTPUT); // b0
	pinMode(16, OUTPUT); // b1
	pinMode(1, OUTPUT); // b2
	pinMode(4, OUTPUT); // b3
	pinMode(5, OUTPUT); // b4
	pinMode(6, OUTPUT); // b5
	pinMode(10, OUTPUT); //b6
	pinMode(11, OUTPUT); //b7
	
	pinMode(8, OUTPUT); // a0
	pinMode(9, OUTPUT); // a1
	pinMode(7, OUTPUT); // a2
	pinMode(0, OUTPUT); // a3
	pinMode(2, OUTPUT); // a4
	
	pinMode(3, OUTPUT); // cs
	
	digitalWrite(15, LOW);
	digitalWrite(16, LOW);
	digitalWrite(1, LOW);
	digitalWrite(4, LOW);
	digitalWrite(5, LOW);
	digitalWrite(6, LOW);
	digitalWrite(10, LOW);
	digitalWrite(11, LOW);
	
	digitalWrite(8, LOW);
	digitalWrite(9, LOW);			
	digitalWrite(7, LOW);
	digitalWrite(0, LOW);
	digitalWrite(2, LOW);
	
	digitalWrite(3, HIGH);

    srand(0);
    
	mos6502 cpu(MemRead, MemWrite);
	
	cpu.Reset();
	cpu.Run(10000000);

	int ts = 0;
	int sec = 0;

	timeval t1,t2;
	long int elaps;

	// main loop, play song
	while(1)
	{
		gettimeofday(&t1, NULL);	

		// trigger IRQ interrupt
		cpu.IRQ();
		
		// execute the player routine
		cpu.Run(0);

		gettimeofday(&t2, NULL);

		// wait 1/50 sec. 
		if(t1.tv_sec == t2.tv_sec) elaps = t2.tv_usec - t1.tv_usec;
		else elaps = 1000000 - t1.tv_usec + t2.tv_usec;
		if(elaps < 20000) usleep(20000 - elaps); // 50Hz refresh rate
		ts++;
	}
	
	return 0;
}
