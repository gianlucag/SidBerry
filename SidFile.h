//============================================================================
// Description : SID file parser
// Author      : Gianluca Ghettini
//============================================================================

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
using namespace std;

#define PSID_MIN_HEADER_LENGTH 118		// Version 1
#define PSID_MAX_HEADER_LENGTH 124		// Version 2

// Offsets of fields in header (all fields big-endian)
enum {
	SIDFILE_PSID_ID = 0,			// 'PSID'
	SIDFILE_PSID_VERSION = 4,		// 1 or 2
	SIDFILE_PSID_LENGTH = 6,		// Header length
	SIDFILE_PSID_START = 8,			// C64 load address
	SIDFILE_PSID_INIT = 10,			// C64 init routine address
	SIDFILE_PSID_MAIN = 12,			// C64 replay routine address
	SIDFILE_PSID_NUMBER = 14,		// Number of subsongs
	SIDFILE_PSID_DEFSONG = 16,		// Main subsong number
	SIDFILE_PSID_SPEED = 18,		// Speed flags (1 bit/song)
	SIDFILE_PSID_NAME = 22,			// Module name (ISO Latin1 character set)
	SIDFILE_PSID_AUTHOR = 54,		// Author name (dto.)
	SIDFILE_PSID_COPYRIGHT = 86,	// Copyright info (dto.)
	SIDFILE_PSID_FLAGS = 118,		// Flags (only in version 2 header)
	SIDFILE_PSID_RESERVED = 120
};

enum {
	SIDFILE_OK,
	SIDFILE_ERROR_FILENOTFOUND,
	SIDFILE_ERROR_MALFORMED
};

enum {
	SIDFILE_SPEED_50HZ,
	SIDFILE_SPEED_60HZ
};


class SidFile
{
private:
	
	string moduleName;
	string authorName;
	string copyrightInfo;
	int numOfSongs;
	int firstSong;
	uint16_t initAddr;
	uint16_t playAddr;
	uint64_t loadAddr;
	uint32_t speedFlags;
	uint8_t dataBuffer[0x10000];
	uint16_t dataLength;
	
	uint16_t Read16(const uint8_t *p, int offset);
	uint32_t Read32(const uint8_t *p, int offset);
	bool IsPSIDHeader(const uint8_t *p);
	
public:
	
	SidFile();
	int Parse(string file);
	string GetModuleName();
	string GetAuthorName();
	string GetCopyrightInfo();
	int GetSongSpeed(int songNum);
	int GetNumOfSongs();
	uint8_t *GetDataPtr();
	uint16_t GetDataLength();
	uint16_t GetLoadAddress();
	uint16_t GetInitAddress();
	uint16_t GetPlayAddress();
};
