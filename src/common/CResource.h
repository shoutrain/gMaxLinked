/*
 ============================================================================
 Name        : CResource.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of resource management
 ============================================================================
 */

#ifndef _C_RESOURCE_H_
#define _C_RESOURCE_H_

#include "CBase.h"
#include "CAutoLock.h"

#include <deque>

class CMutex;

template<class T, class Y>
class CResource: public CBase {
public:

	CResource(const unsigned int maxNum, CMutex *mutex = NULL) :
			_maxNum(maxNum) {
		assert(_maxNum > 1);
		_unitGroup = (T **) new char[_maxNum * sizeof(T *)];
		_mutex = mutex;

		for (unsigned int ui = 0; ui < _maxNum; ui++) {
			_unitGroup[ui] = new T();
			_freeDeque.push_back(_unitGroup[ui]);
		}
	}

	CResource(const unsigned int maxNum, Y *container, CMutex *mutex = NULL) :
			_maxNum(maxNum) {
		assert(_maxNum > 1);
		assert(NULL != container);
		_unitGroup = (T **) new char[_maxNum * sizeof(T *)];
		_mutex = mutex;

		for (unsigned int ui = 0; ui < _maxNum; ui++) {
			_unitGroup[ui] = new T(container);
			_freeDeque.push_back(_unitGroup[ui]);
		}
	}

	virtual ~CResource() {
		_freeDeque.clear();

		for (unsigned int ui = 0; ui < _maxNum; ui++) {
			delete _unitGroup[ui];
		}

		delete[] _unitGroup;
	}

	T *allocate() {
		CAutoLock al(_mutex);

		if (0 < _freeDeque.size()) {
			T *pUnit = _freeDeque.front();

			assert(pUnit);
			_freeDeque.pop_front();

			return pUnit;
		}

		return NULL;
	}

	bool reclaim(T *&unit) {
		assert(unit);
		CAutoLock al(_mutex);

		unsigned int ui = 0;

		for (; ui < _maxNum; ui++) {
			if (unit == _unitGroup[ui]) {
				break;
			}
		}

		if (ui != _maxNum) {
			_freeDeque.push_back(unit);
			unit = NULL;

			return true;
		}

		return false;
	}

	unsigned int size() const {
		return (unsigned int) _freeDeque.size();
	}

private:
	const unsigned int _maxNum;
	T **_unitGroup;
	CMutex *_mutex;

	std::deque<T*> _freeDeque;
};

#endif // _C_RESOURCE_H_
