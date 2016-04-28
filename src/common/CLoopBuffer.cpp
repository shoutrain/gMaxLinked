/*
 ============================================================================
 Name        : CLoopBuffer.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of loop buffer
 ============================================================================
 */

#include "CLoopBuffer.h"
#include "CMutex.h"
#include "CAutoLock.h"

#include <string.h>

CLoopBuffer::CLoopBuffer(unsigned int size, CMutex *mutex, bool isPadding) :
		_totalSize(size) {
	assert(size);
	_actualSize = size;
	_usedSize = 0;
	_freeSize = size;
	_mutex = mutex;
	_isPadding = isPadding;
	_padding = 0;
	_writePoint = _readPoint = _buffer = new unsigned char[size];
	assert(_buffer);
}

CLoopBuffer::~CLoopBuffer() {
	delete[] _buffer;
}

bool CLoopBuffer::write(const unsigned char *buffer, unsigned int size) {
	assert(buffer);
	assert(size);

	CAutoLock al(_mutex);

	if (size > _freeSize) {
		return false;
	}

	if (_writePoint >= _readPoint) {
		unsigned int rightSize = _buffer + _totalSize - _writePoint;
		unsigned int leftSize = _readPoint - _buffer;

		assert(_padding == 0);
		assert(_actualSize == _totalSize);
		assert(_freeSize <= _totalSize);
		assert(_writePoint - _readPoint == (int ) _usedSize);
		assert(rightSize + leftSize == _freeSize);

		if (rightSize >= size) {
			memcpy(_writePoint, buffer, size);
			_writePoint += size;
		} else if (_isPadding) {
			if (size > leftSize) {
				return false;
			}

			_padding = rightSize;
			_actualSize -= _padding;
			_freeSize -= _padding;
			assert(_freeSize == leftSize);

			memcpy(_buffer, buffer, size);
			_writePoint = _buffer + size;
		} else {
			memcpy(_writePoint, buffer, rightSize);
			memcpy(_buffer, buffer + rightSize, size - rightSize);
			_writePoint = _buffer + size - rightSize;
		}
	} else {
		assert(_readPoint - _writePoint == (int ) _freeSize);
		memcpy(_writePoint, buffer, size);
		_writePoint += size;
		assert(_writePoint <= _readPoint);
	}

	_usedSize += size;
	_freeSize -= size;

	return true;
}

unsigned int CLoopBuffer::read(unsigned char *buffer, unsigned int size) {
	assert(buffer);
	assert(size);

	CAutoLock al(_mutex);

	if (0 == _usedSize) {
		return 0;
	}

	if (_writePoint > _readPoint) {
		assert(_padding == 0);
		assert(_writePoint - _readPoint == (int ) _usedSize);

		if (size > _usedSize) {
			size = _usedSize;
		}

		memcpy(buffer, _readPoint, size);
		_readPoint += size;
	} else {
		assert(_readPoint - _writePoint == (int ) _freeSize);

		unsigned int uiRightSize = _buffer + _actualSize - _readPoint;
		unsigned int uiLeftSize = _writePoint - _buffer;

		assert(_actualSize == _totalSize - _padding);
		assert(uiRightSize + uiLeftSize == _usedSize);

		if (uiRightSize >= size) {
			memcpy(buffer, _readPoint, size);
			_readPoint += size;

			if (uiRightSize == size && _padding) {
				_actualSize += _padding;
				_freeSize += _padding;
				_padding = 0;
				_readPoint = _buffer;
			}
		} else {
			if (_usedSize < size) {
				size = _usedSize;
			}

			memcpy(buffer, _readPoint, uiRightSize);
			memcpy(buffer + uiRightSize, _buffer, size - uiRightSize);
			_readPoint = _buffer + size - uiRightSize;

			if (_padding) {
				_actualSize += _padding;
				_freeSize += _padding;
				_padding = 0;
			}
		}
	}

	_usedSize -= size;
	_freeSize += size;

	return size;
}

void CLoopBuffer::reset() {
	CAutoLock al(_mutex);

	_actualSize = _totalSize;
	_usedSize = 0;
	_freeSize = _totalSize;
	_padding = 0;
	_writePoint = _readPoint = _buffer;
}

