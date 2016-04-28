/*
 ============================================================================
 Name        : CIniReader.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : ini file reader
 ============================================================================
 */

#ifndef _C_INI_READER_H_
#define _C_INI_READER_H_

#include "CBase.h"

const unsigned int MAX_CONFIG_FILE_SIZE = 0x8000;

class CIniReader: CBase {
public:
	CIniReader(const char *fileName);
	virtual ~CIniReader();

	int readString(const char *section, const char *key, char *value,
			unsigned int size);
	int readByte(const char *section, const char *key, unsigned char *value);
	int readShort(const char *section, const char *key, unsigned short *value);
	int readInt(const char *section, const char *key, unsigned int *value);

private:
	int isNewline(char c);
	int isEndOfString(char c);
	int isLeftBarce(char c);
	int isRightBrace(char c);
	int parseFile(const char *section, const char *key, int *secS, int *secE,
			int *keyS, int *keyE, int *valueS, int *valueE);

	char _buffer[MAX_CONFIG_FILE_SIZE];
	int _bufferSize;
};

#endif // _C_INI_READER_H_
