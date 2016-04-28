/*
 ============================================================================
 Name        : CLoopBuffer.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of loop buffer
 ============================================================================
 */

#ifndef _C_LOOP_BUFFER_H_
#define _C_LOOP_BUFFER_H_

#include "CBase.h"

class CMutex;

class CLoopBuffer: public CBase {
public:
	CLoopBuffer(unsigned int size, CMutex *mutex = NULL,
			bool isPadding = false);
	virtual ~CLoopBuffer();

	// buffer: the buffer to be write to the LoopBuffer
	// size: the buffer size
	// Return true if success, otherwise false
	bool write(const unsigned char *buffer, unsigned int size);

	// buffer: the buffer to be read in
	// size: the buffer size
	// Return actual read size
	unsigned int read(unsigned char *buffer, unsigned int size);

	void reset();

	unsigned int getTotalSize() const {
		return _totalSize;
	}

	unsigned int getActualSize() const {
		return _actualSize;
	}

	unsigned int getFreeSize() const {
		return _freeSize;
	}

	unsigned int getUsedSize() const {
		return _usedSize;
	}

protected:
	unsigned char *_buffer;

private:
	const unsigned int _totalSize;
	unsigned int _actualSize; // _totalSize - _padding
	unsigned int _usedSize; // _actualSize - _freeSize
	unsigned int _freeSize; // _actualSize - _userSize

	bool _isPadding;
	unsigned int _padding; // = _totalSize - _actualSize

	unsigned char *_writePoint;
	unsigned char *_readPoint;

	CMutex *_mutex;
};

#endif // _C_LOOP_BUFFER_H_
