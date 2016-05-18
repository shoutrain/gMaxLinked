/*
 ============================================================================
 Name        : CConfReader.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : configure file reader
 ============================================================================
 */

#ifndef _C_CONF_READER_H_
#define _C_CONF_READER_H_

#include "CBase.h"

const ub4_ MAX_CONFIG_FILE_SIZE = 0x8000;

class CConfReader: CBase {
public:
	CConfReader(const c1_ *fileName);
	virtual ~CConfReader();

	b4_ readString(const c1_ *section, const c1_ *key, c1_ *value, ub4_ size);
	b4_ readByte(const c1_ *section, const c1_ *key, ub1_ *value);
	b4_ readShort(const c1_ *section, const c1_ *key, ub2_ *value);
	b4_ readInt(const c1_ *section, const c1_ *key, ub4_ *value);

private:
	b4_ _isNewline(c1_ c);
	b4_ _isEndOfString(c1_ c);
	b4_ _isLeftBarce(c1_ c);
	b4_ _isRightBrace(c1_ c);
	b4_ _parseFile(const c1_ *section, const c1_ *key, b4_ *secS, b4_ *secE,
			b4_ *keyS, b4_ *keyE, b4_ *valueS, b4_ *valueE);

	c1_ _buffer[MAX_CONFIG_FILE_SIZE];
	b4_ _bufferSize;
};

#endif // _C_CONF_READER_H_
