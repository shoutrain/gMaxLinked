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

CLoopBuffer::CLoopBuffer(ub4_ size, CMutex *mutex, bool_ isPadding) :
		_totalSize(size) {
	assert(size);
	_actualSize = size;
	_usedSize = 0;
	_freeSize = size;
	_mutex = mutex;
	_isPadding = isPadding;
	_padding = 0;
	_writePob4_ = _readPob4_ = __buffer = new ub1_[size];
	assert(__buffer);
}

CLoopBuffer::~CLoopBuffer() {
	delete[] __buffer;
}

bool_ CLoopBuffer::write(const ub1_ *buffer, ub4_ size) {
	assert(buffer);
	assert(size);

	CAutoLock al(_mutex);

	if (size > _freeSize) {
		return false_v;
	}

	if (_writePob4_ >= _readPob4_) {
		ub4_ rightSize = __buffer + _totalSize - _writePob4_;
		ub4_ leftSize = _readPob4_ - __buffer;

		assert(_padding == 0);
		assert(_actualSize == _totalSize);
		assert(_freeSize <= _totalSize);
		assert(_writePob4_ - _readPob4_ == (b4_ ) _usedSize);
		assert(rightSize + leftSize == _freeSize);

		if (rightSize >= size) {
			memcpy(_writePob4_, buffer, size);
			_writePob4_ += size;
		} else if (_isPadding) {
			if (size > leftSize) {
				return false_v;
			}

			_padding = rightSize;
			_actualSize -= _padding;
			_freeSize -= _padding;
			assert(_freeSize == leftSize);

			memcpy(__buffer, buffer, size);
			_writePob4_ = __buffer + size;
		} else {
			memcpy(_writePob4_, buffer, rightSize);
			memcpy(__buffer, buffer + rightSize, size - rightSize);
			_writePob4_ = __buffer + size - rightSize;
		}
	} else {
		assert(_readPob4_ - _writePob4_ == (b4_ ) _freeSize);
		memcpy(_writePob4_, buffer, size);
		_writePob4_ += size;
		assert(_writePob4_ <= _readPob4_);
	}

	_usedSize += size;
	_freeSize -= size;

	return true_v;
}

ub4_ CLoopBuffer::read(ub1_ *buffer, ub4_ size) {
	assert(buffer);
	assert(size);

	CAutoLock al(_mutex);

	if (0 == _usedSize) {
		return 0;
	}

	if (_writePob4_ > _readPob4_) {
		assert(_padding == 0);
		assert(_writePob4_ - _readPob4_ == (b4_ ) _usedSize);

		if (size > _usedSize) {
			size = _usedSize;
		}

		memcpy(buffer, _readPob4_, size);
		_readPob4_ += size;
	} else {
		assert(_readPob4_ - _writePob4_ == (b4_ ) _freeSize);

		ub4_ uiRightSize = __buffer + _actualSize - _readPob4_;
		ub4_ uiLeftSize = _writePob4_ - __buffer;

		assert(_actualSize == _totalSize - _padding);
		assert(uiRightSize + uiLeftSize == _usedSize);

		if (uiRightSize >= size) {
			memcpy(buffer, _readPob4_, size);
			_readPob4_ += size;

			if (uiRightSize == size && _padding) {
				_actualSize += _padding;
				_freeSize += _padding;
				_padding = 0;
				_readPob4_ = __buffer;
			}
		} else {
			if (_usedSize < size) {
				size = _usedSize;
			}

			memcpy(buffer, _readPob4_, uiRightSize);
			memcpy(buffer + uiRightSize, __buffer, size - uiRightSize);
			_readPob4_ = __buffer + size - uiRightSize;

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

none_ CLoopBuffer::reset() {
	CAutoLock al(_mutex);

	_actualSize = _totalSize;
	_usedSize = 0;
	_freeSize = _totalSize;
	_padding = 0;
	_writePob4_ = _readPob4_ = __buffer;
}

