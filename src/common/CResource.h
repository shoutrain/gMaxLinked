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

#include <deque>

#include "CAutoLock.h"
#include "CBase.h"

class CMutex;

template <class T, class Y>
class CResource : public CBase {
public:
    CResource(const ub4_ maxNum, CMutex *mutex = null_v) : _maxNum(maxNum) {
        assert(_maxNum > 1);
        _unitGroup = (T **)new b1_[_maxNum * sizeof(T *)];
        _mutex = mutex;

        for (ub4_ ui = 0; ui < _maxNum; ui++) {
            _unitGroup[ui] = new T();
            _freeDeque.push_back(_unitGroup[ui]);
        }
    }

    CResource(const ub4_ maxNum, Y *container, CMutex *mutex = null_v)
        : _maxNum(maxNum) {
        assert(_maxNum > 1);
        assert(null_v != container);
        _unitGroup = (T **)new b1_[_maxNum * sizeof(T *)];
        _mutex = mutex;

        for (ub4_ ui = 0; ui < _maxNum; ui++) {
            _unitGroup[ui] = new T(container);
            _freeDeque.push_back(_unitGroup[ui]);
        }
    }

    virtual ~CResource() {
        _freeDeque.clear();

        for (ub4_ ui = 0; ui < _maxNum; ui++) {
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

        return null_v;
    }

    bool_ reclaim(T *&unit) {
        assert(unit);
        CAutoLock al(_mutex);

        ub4_ ui = 0;

        for (; ui < _maxNum; ui++) {
            if (unit == _unitGroup[ui]) {
                break;
            }
        }

        if (ui != _maxNum) {
            _freeDeque.push_back(unit);
            unit = null_v;

            return true_v;
        }

        return false_v;
    }

    ub4_ size() const { return (ub4_)_freeDeque.size(); }

private:
    const ub4_ _maxNum;
    T **_unitGroup;
    CMutex *_mutex;

    std::deque<T *> _freeDeque;
};

#endif  // _C_RESOURCE_H_
