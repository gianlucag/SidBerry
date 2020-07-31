//============================================================================
// Description : SidBerry player & driver 
// Author      : Gianluca Ghettini, Alessio Lombardo
//============================================================================

#include <iostream>
using namespace std;
#include <stdlib.h>
#include "mos6502.h"
#include "SidFile.h"
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "gpioInterface.h"

uint8_t memory[65536]; // 64K ram
bool verbose = false;

void TestWrite(uint16_t addr, uint8_t byte)
{	
		printf("\na: %02X, b: %02X", addr, byte);		
		
		uint16_t phyaddr = addr;
		
		// set address to the bus
		if(phyaddr & 0x01) gpioWrite(A0, HIGH);
		else gpioWrite(A0, LOW);
		if(phyaddr & 0x02) gpioWrite(A1, HIGH);
		else gpioWrite(A1, LOW);
		if(phyaddr & 0x04) gpioWrite(A2, HIGH);
		else gpioWrite(A2, LOW);
		if(phyaddr & 0x08) gpioWrite(A3, HIGH);
		else gpioWrite(A3, LOW);
		if(phyaddr & 0x10) gpioWrite(A4, HIGH);
		else gpioWrite(A4, LOW);
		
		// set data to the bus
		if(byte & 0x01) gpioWrite(D0, HIGH);
		else gpioWrite(D0, LOW);
		if(byte & 0x02) gpioWrite(D1, HIGH);
		else gpioWrite(D1, LOW);
		if(byte & 0x04) gpioWrite(D2, HIGH);
		else gpioWrite(D2, LOW);
		if(byte & 0x08) gpioWrite(D3, HIGH);
		else gpioWrite(D3, LOW);
		if(byte & 0x10) gpioWrite(D4, HIGH);
		else gpioWrite(D4, LOW);			
		if(byte & 0x20) gpioWrite(D5, HIGH);
		else gpioWrite(D5, LOW);
		if(byte & 0x40) gpioWrite(D6, HIGH);
		else gpioWrite(D6, LOW);
		if(byte & 0x80) gpioWrite(D7, HIGH);
		else gpioWrite(D7, LOW);		
		
		// assert cs line
		gpioWrite(CS, LOW);
		usleep(500000);		
		gpioWrite(CS, HIGH);	

	printf("\nOK\n");

	return;
}

void MemWrite(uint16_t addr, uint8_t byte)
{	
	if(addr >= 0xD400 && addr <= 0xD7FF) // address decoding login
	{
		// access to SID chip
		
		memory[addr] = byte;

		if(verbose){
			//NOTE: If you use a slow connection to tty device, the printf function may affect the playback speed
			printf("%02X%02X %02X%02X %02X %02X %02X %02X%02X %02X%02X %02X %02X %02X %02X%02X %02X%02X %02X %02X %02X\n",
					memory[0xD400], memory[0xD401], memory[0xD402], memory[0xD403],memory[0xD404], memory[0xD405], memory[0xD406],
					memory[0xD407], memory[0xD408], memory[0xD409], memory[0xD40A],memory[0xD40B], memory[0xD40C], memory[0xD40D],
					memory[0xD40E], memory[0xD40F], memory[0xD410], memory[0xD411],memory[0xD412], memory[0xD413], memory[0xD414]
			);
		}
		
		uint16_t phyaddr = addr & 0x1f;
		
		// set address to the bus
		if(phyaddr & 0x01) gpioWrite(A0, HIGH);
		else gpioWrite(A0, LOW);
		if(phyaddr & 0x02) gpioWrite(A1, HIGH);
		else gpioWrite(A1, LOW);
		if(phyaddr & 0x04) gpioWrite(A2, HIGH);
		else gpioWrite(A2, LOW);
		if(phyaddr & 0x08) gpioWrite(A3, HIGH);
		else gpioWrite(A3, LOW);
		if(phyaddr & 0x10) gpioWrite(A4, HIGH);
		else gpioWrite(A4, LOW);
		
		// set data to the bus
		if(byte & 0x01) gpioWrite(D0, HIGH);
		else gpioWrite(D0, LOW);
		if(byte & 0x02) gpioWrite(D1, HIGH);
		else gpioWrite(D1, LOW);
		if(byte & 0x04) gpioWrite(D2, HIGH);
		else gpioWrite(D2, LOW);
		if(byte & 0x08) gpioWrite(D3, HIGH);
		else gpioWrite(D3, LOW);
		if(byte & 0x10) gpioWrite(D4, HIGH);
		else gpioWrite(D4, LOW);			
		if(byte & 0x20) gpioWrite(D5, HIGH);
		else gpioWrite(D5, LOW);
		if(byte & 0x40) gpioWrite(D6, HIGH);
		else gpioWrite(D6, LOW);
		if(byte & 0x80) gpioWrite(D7, HIGH);
		else gpioWrite(D7, LOW);		
		
		// assert CS line (strobe)
		gpioWrite(CS, LOW);
		usleep(1);		
		gpioWrite(CS, HIGH);	
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
			// emulate read access to OSC3/Random register, return a random value
			printf("\nread! 1b");
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

int load_sid(mos6502 cpu, SidFile sid, int song_number){
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
	memory[0x0001] = song_number;
	
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
	
	// setup wiring library, configure GPIOs
	gpioSetup();
	
	gpioMode(D0, OUTPUT);
	gpioMode(D1, OUTPUT);
	gpioMode(D2, OUTPUT);
	gpioMode(D3, OUTPUT);
	gpioMode(D4, OUTPUT);
	gpioMode(D5, OUTPUT);
	gpioMode(D6, OUTPUT);
	gpioMode(D7, OUTPUT);
	
	gpioMode(A0, OUTPUT);
	gpioMode(A1, OUTPUT);
	gpioMode(A2, OUTPUT);
	gpioMode(A3, OUTPUT);
	gpioMode(A4, OUTPUT);
	
	gpioMode(CS, OUTPUT);
	
	gpioWrite(D0, LOW);
	gpioWrite(D1, LOW);
	gpioWrite(D2, LOW);
	gpioWrite(D3, LOW);
	gpioWrite(D4, LOW);
	gpioWrite(D5, LOW);
	gpioWrite(D6, LOW);
	gpioWrite(D7, LOW);
	
	gpioWrite(A0, LOW);
	gpioWrite(A1, LOW);	
	gpioWrite(A2, LOW);
	gpioWrite(A3, LOW);
	gpioWrite(A4, LOW);

	gpioWrite(CS, HIGH);
	
	cpu.Reset();
	//cpu.Run(10000000);
	cpu.Run(100000);

}

int getch_noecho_special_char() {
	
	int char_code = 0;
	int buf = 0;
	char buf2[3] = {0,0,0};
	const char* buf_erase_echo = string("\033[2K\r").data();
	
    struct termios old = {0};
    tcgetattr(0, &old);
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 0;
    old.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &old);
	
	read(0, &buf2[0], 1);
	read(0, &buf2[1], 1);
	read(0, &buf2[2], 1);
	if(buf2[0]!=0) write(0, buf_erase_echo, 5);

	if(buf2[0]=='\033' && buf2[1]=='\0'){ //Escape Key
		buf=256;
	}else if(buf2[0]=='\033' && buf2[1]==91 && buf2[2]==68 ){ //Left Arrow
		buf=257;
	}else if(buf2[0]=='\033' && buf2[1]==91 && buf2[2]==67 ){ //Right Arrow
		buf=258;
	}else{
		buf=buf2[0];
	}
				
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    tcsetattr(0, TCSADRAIN, &old);
	
    return buf;
	
}

void change_player_status(mos6502 cpu, SidFile sid, int key_press, bool* paused, bool* exit, uint8_t* mode_vol_reg, int* song_number, int* sec, int* min){
	if(key_press==256 || key_press==(int)'q'){ //Escape (reset all registers and quit)
		printf("Exit\n");
		for(int i=0xD400;i<0xD414;i++){
			MemWrite(i, 0);
		}
		*paused=false;
		*exit=true;
	}else if(key_press==32){ //Pause
		if(*paused){
			printf("\rPlay Sub-Song %d / %d [%02d:%02d]",(*song_number)+1,sid.GetNumOfSongs(),*min,*sec); fflush(stdout);
			MemWrite(0xD418, *mode_vol_reg);		
			*paused=false;		
		}else{
			printf("\rPlay Sub-Song %d / %d [%02d:%02d][PAUSE]",(*song_number)+1,sid.GetNumOfSongs(),*min,*sec); fflush(stdout);	
			*mode_vol_reg = MemRead(0xD418);		
			MemWrite(0xD418, 0);		
			*paused=true;	
		}
	}else if(key_press==(int)'v'){
		verbose = !verbose;
		if(verbose) cout << "VERBOSE" << endl;
		else cout << "NO VERBOSE" << endl;
	}else if(key_press==(int)'r'){
		load_sid(cpu,sid,*song_number);
		*min=0;
		*sec=0;
		*paused=false;
		printf("\rPlay Sub-Song %d / %d [%02d:%02d]",(*song_number)+1,sid.GetNumOfSongs(),*min,*sec); fflush(stdout);
	}else if(key_press==257){ //Previous Sub-Song
		(*song_number)--;
		if(*song_number<0) *song_number=sid.GetNumOfSongs()-1;
		load_sid(cpu,sid,*song_number);	
		*min=0;
		*sec=0;
		*paused=false;
		printf("\rPlay Sub-Song %d / %d [%02d:%02d]",(*song_number)+1,sid.GetNumOfSongs(),*min,*sec); fflush(stdout);
	}else if(key_press==258){ //Next Sub-Song
		(*song_number)++;
		if(*song_number==sid.GetNumOfSongs()) (*song_number)=0;
		load_sid(cpu,sid,*song_number);
		*min=0;
		*sec=0;
		*paused=false;		
		printf("\rPlay Sub-Song %d / %d [%02d:%02d]",(*song_number)+1,sid.GetNumOfSongs(),*min,*sec); fflush(stdout);
	}else if(key_press>0){
		if(*paused){
			printf("\rPlay Sub-Song %d / %d [%02d:%02d][PAUSE]",(*song_number)+1,sid.GetNumOfSongs(),*min,*sec); fflush(stdout);
		}else{
			printf("\rPlay Sub-Song %d / %d [%02d:%02d]",(*song_number)+1,sid.GetNumOfSongs(),*min,*sec); fflush(stdout);		
		}
	}
	
}



int main(int argc, char *argv[])
{
	SidFile sid;
	
	string filename = "";
    int song_number = 0;
	
	for(int param_count = 1;param_count<argc;param_count++){
		if(filename.length()==0 and argv[param_count][0]!='-'){
			filename=argv[param_count];			
		}else if(!strcmp(argv[param_count],"-v") || !strcmp(argv[param_count],"--verbose")){
			verbose = true;
		}else if(!strcmp(argv[param_count],"-V") || !strcmp(argv[param_count],"--version")){
			cout << "SidBerry 3.0 - (July 2020)" << endl;
			cout << "MOS SID (MOS6581/8580) Player for RaspberryPI, AriettaG25 and others Linux-based systems with GPIO ports" <<endl;
			cout << "Hardware for RaspberryPI : Gianluca Ghettini, Thoroide "<< endl;
			cout << "Hardware for AriettaG25  : Alessio Lombardo " << endl;
			cout << "Low-level interface      : Gianluca Ghettini, Alessio Lombardo " << endl;
			cout << "MOS6502 Emulator         : Gianluca Ghettini" << endl;
			cout << "Sid Player               : Gianluca Ghettini, Alessio Lombardo " << endl;
		}else if(!strcmp(argv[param_count],"-s") || !strcmp(argv[param_count],"--song")){
			param_count++;
			song_number = atoi(argv[param_count])-1;
		}else if(!strcmp(argv[param_count],"-h") || !strcmp(argv[param_count],"--help")){
			cout << "Usage: " << argv[0] << " <Sid Filename> [Options]" << endl;
			cout << "Options: " << endl;
			cout << " -s, --song    : Set Sub-Song number (default depends on the Sid File) " << endl;			
			cout << " -v, --verbose : Verbose mode (show SID registers content) " << endl;
			cout << " -V, --version : Show version and other informations " << endl;
			cout << " -h, --help    : Show this help message " << endl;
			return 0;
		}else{
			cout << "Warning: Invalid Parameter at position " << param_count << endl;
		}
		
	}
	
	int res = sid.Parse(filename);
	if(song_number<0 or song_number>=sid.GetNumOfSongs()){
		cout << "Warning: Invalid Sub-Song Number. Default Sub-Song will be chosen." << endl;
		song_number=sid.GetFirstSong();	
	}
	
	if(res == SIDFILE_ERROR_FILENOTFOUND)
	{
		cerr << "error loading sid file! not found" << endl;
		return 1;
	}
	
	if(res == SIDFILE_ERROR_MALFORMED)
	{
		cerr << "error loading sid file! malformed" << endl;
		return 2;
	}
	cout << "\n< Sid Info >" << endl;	
	cout << "Module Name       : " << sid.GetModuleName() << endl ;
	cout << "Author Name       : " << sid.GetAuthorName() << endl ;
	cout << "Copyright         : " << sid.GetCopyrightInfo() << endl ;
	
	cout << "Load Address      : " << sid.GetLoadAddress() << endl ;
	cout << "Init Address      : " << sid.GetInitAddress() << endl ;
	cout << "Play Address      : " << sid.GetPlayAddress() << endl ;

	cout << "Selected Sub-Song : " << song_number+1 << " / " << sid.GetNumOfSongs() << endl ;

    srand(0);
	mos6502 cpu(MemRead, MemWrite);
	
	int time_unit = 0;
	int sec = 0;
	int min = 0;

	timeval t1,t2;
	long int elaps;

	load_sid(cpu,sid,song_number);
	
	if(verbose) cout << endl;
	
	cout << "\n< Player Commands >" << endl;
	cout << "Space       : Pause/Continue " << endl;
	cout << "Left  Arrow : Previous Sub-Song " << endl;
	cout << "Right Arrow : Next Sub-Song " << endl;	
	cout << "R           : Restart current Sub-Song " << endl;	
	cout << "V           : Verbose (show SID registers) " << endl;	
	cout << "Q or Escape : Quit " << endl << endl;
	
	printf("\rPlay Sub-Song %d / %d [%02d:%02d]",song_number+1,sid.GetNumOfSongs(),min,sec); fflush(stdout);
	
	bool paused = false;
	bool exit = false;
	uint8_t mode_vol_reg = 0;

	
	// main loop, play song
	while(!exit)
	{
		
		while(paused){	
			change_player_status(cpu, sid, getch_noecho_special_char(),&paused,&exit,&mode_vol_reg,&song_number, &sec, &min);
			usleep(100000);
		}
		
		gettimeofday(&t1, NULL);	

		change_player_status(cpu, sid, getch_noecho_special_char(),&paused,&exit,&mode_vol_reg,&song_number, &sec, &min);
		
		if(exit) break;
		if(paused) continue;

		
		// trigger IRQ interrupt
		cpu.IRQ();
		
		// execute the player routine
		cpu.Run(0);
		
		gettimeofday(&t2, NULL);

		// wait 1/50 sec. 
		if(t1.tv_sec == t2.tv_sec) elaps = t2.tv_usec - t1.tv_usec;
		else elaps = 1000000 - t1.tv_usec + t2.tv_usec;
		if(elaps < 20000) usleep(20000 - elaps); // 50Hz refresh rate
		time_unit++;		
		if(time_unit%50==0){
			sec++;
			if(sec==60){
				sec=0;
				min++;
			}
			if(!verbose){
				printf("\rPlay Sub-Song %d / %d [%02d:%02d]",song_number+1,sid.GetNumOfSongs(),min,sec); fflush(stdout);
			}
		}
	}
	
	return 0;
}
