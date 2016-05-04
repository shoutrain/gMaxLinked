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

const ub4_ MAX_CONFIG_FILE_SIZE = 0x8000;

class CIniReader: CBase {
public:
	CIniReader(const s1_ fileName);
	virtual ~CIniReader();

	b4_ readString(const s1_ section, const s1_ key, s1_ value, ub4_ size);
	b4_ readByte(const s1_ section, const s1_ key, ub1_ *value);
	b4_ readShort(const s1_ section, const s1_ key, ub2_ *value);
	b4_ readInt(const s1_ section, const s1_ key, ub4_ *value);

private:
	b4_ isNewline(c1_ c);
	b4_ isEndOfString(c1_ c);
	b4_ isLeftBarce(c1_ c);
	b4_ isRightBrace(c1_ c);
	b4_ parseFile(const s1_ section, const s1_ key, b4_ *secS, b4_ *secE,
			b4_ *keyS, b4_ *keyE, b4_ *valueS, b4_ *valueE);

	c1_ _buffer[MAX_CONFIG_FILE_SIZE];
	b4_ _bufferSize;
};

#endif // _C_INI_READER_H_
