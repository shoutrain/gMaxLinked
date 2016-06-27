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
	CLoopBuffer(ub4_ size, CMutex *mutex = null_v, bool_ isPadding = false_v);
	virtual ~CLoopBuffer();

	// buffer: the buffer to be write to the LoopBuffer
	// size: the buffer size
	// Return true_v if success, otherwise false_v
	bool_ write(const ub1_ *buffer, ub4_ size);

	// buffer: the buffer to be read in
	// size: the buffer size
	// Return actual read size
	ub4_ read(ub1_ *buffer, ub4_ size);

	none_ reset();

	ub4_ getTotalSize() const {
		return _totalSize;
	}

	ub4_ getActualSize() const {
		return _actualSize;
	}

	ub4_ getFreeSize() const {
		return _freeSize;
	}

	ub4_ getUsedSize() const {
		return _usedSize;
	}

protected:
	ub1_ *__buffer;

private:
	const ub4_ _totalSize;
	ub4_ _actualSize; // _totalSize - _padding
	ub4_ _usedSize; // _actualSize - _freeSize
	ub4_ _freeSize; // _actualSize - _userSize

	bool_ _isPadding;
	ub4_ _padding; // = _totalSize - _actualSize

	ub1_ *_writePos;
	ub1_ *_readPos;

	CMutex *_mutex;
};

#endif // _C_LOOP_BUFFER_H_
