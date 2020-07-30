#include "SidFile.h"

SidFile::SidFile()
{
	
}

uint16_t SidFile::Read16(const uint8_t *p, int offset)
{
	return (p[offset] << 8) | p[offset + 1];
}

uint32_t SidFile::Read32(const uint8_t *p, int offset)
{
	return (p[offset] << 24) | (p[offset + 1] << 16) | (p[offset + 2] << 8) | p[offset + 3];
}

bool SidFile::IsPSIDHeader(const uint8_t *p)
{
	uint32_t id = Read32(p, SIDFILE_PSID_ID);
	uint16_t version = Read16(p, SIDFILE_PSID_VERSION);
	return id == 0x50534944 && (version == 1 || version == 2);
}

int SidFile::Parse(string file)
{
	FILE *f = fopen(file.c_str(), "rb");
	
	if(f == NULL)
	{
		return SIDFILE_ERROR_FILENOTFOUND;
	}
	
	uint8_t header[PSID_MAX_HEADER_LENGTH];
	memset(header, 0, PSID_MAX_HEADER_LENGTH);
	
	size_t read = fread(header, 1, PSID_MAX_HEADER_LENGTH, f);
	
	if(read < PSID_MIN_HEADER_LENGTH || !IsPSIDHeader(header))
	{
		fclose(f);
		return SIDFILE_ERROR_MALFORMED;
	}

	numOfSongs = Read16(header, SIDFILE_PSID_NUMBER);
	
	if(numOfSongs == 0)
	{
		numOfSongs = 1;
	}
	
	firstSong = Read16(header, SIDFILE_PSID_DEFSONG);
	if(firstSong)
	{
		firstSong--;
	}
	if (firstSong >= numOfSongs)
	{
		firstSong = 0;
	}

	initAddr = Read16(header, SIDFILE_PSID_INIT);
	playAddr = Read16(header, SIDFILE_PSID_MAIN);
	speedFlags = Read32(header, SIDFILE_PSID_SPEED);

	moduleName = (char *)(header + SIDFILE_PSID_NAME);
	
	authorName = (char *)(header + SIDFILE_PSID_AUTHOR);
	
	string copyrightInfo = (char *)(header + SIDFILE_PSID_COPYRIGHT);

	// Seek to start of module data
	fseek(f, Read16(header, SIDFILE_PSID_LENGTH), SEEK_SET);

	// Find load address
	loadAddr = Read16(header, SIDFILE_PSID_START);
	if(loadAddr == 0)
	{
		uint8_t lo = fgetc(f);
		uint8_t hi = fgetc(f);
		loadAddr = (hi << 8) | lo;
	}
	
	if(initAddr == 0)
	{
		initAddr = loadAddr;
	}

	// Load module data
	dataLength = fread(dataBuffer, 1, 0x10000, f);
	
	fclose(f);

	return SIDFILE_OK;
}

string SidFile::GetModuleName()
{
	return moduleName;
}

string SidFile::GetAuthorName()
{
	return authorName;
}

string SidFile::GetCopyrightInfo()
{
	return copyrightInfo;
}

int SidFile::GetSongSpeed(int songNum)
{
	return (speedFlags & (1 << songNum)) ? SIDFILE_SPEED_60HZ : SIDFILE_SPEED_50HZ;
}

int SidFile::GetNumOfSongs()
{
	return numOfSongs;
}

int SidFile::GetFirstSong()
{
	return firstSong;
}

uint8_t *SidFile::GetDataPtr()
{
	return dataBuffer;
}

uint16_t SidFile::GetDataLength()
{
	return dataLength;
}

uint16_t SidFile::GetLoadAddress()
{
	return loadAddr;
}

uint16_t SidFile::GetInitAddress()
{
	return initAddr;
}

uint16_t SidFile::GetPlayAddress()
{
	return playAddr;
}

