/*
 ============================================================================
 Name        : CAutoPtr.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of auto-pointer
 ============================================================================
 */


#ifndef _C_AUTO_PTR_H_
#define _C_AUTO_PTR_H_

#include "CBase.h"

template<class T>
class CAutoPtr: public CBase {
public:

	explicit CAutoPtr(T *p = NULL) :
			_p(p) {
	}

	CAutoPtr(CAutoPtr<T> &ptr) :
			_p(ptr.release()) {
	}

	virtual ~CAutoPtr() {
		delete _p;
	}

	T *release() {
		T *p = _p;

		_p = NULL;

		return p;
	}

	const T *get() const {
		return _p;
	}

	void reset(const T *p = NULL) {
		if (p != _p) {
			delete _p;
			_p = p;
		}
	}

	CAutoPtr<T> &operator =(CAutoPtr<T> &ptr) {
		if (this != &ptr) {
			reset(ptr.release());
		}

		return *this;
	}

	T &operator *() const {
		return *_p;
	}

	T *operator ->() const {
		return _p;
	}

private:
	// Cannot new this class
	void *operator new(size_t size);
	T *_p;
};

#endif // _C_AUTO_PTR_H_
