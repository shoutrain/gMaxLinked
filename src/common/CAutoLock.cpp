/*
 ============================================================================
 Name        : CAutoLock.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : helper for auto-locker
 ============================================================================
 */

#include "CAutoLock.h"

CAutoLock::CAutoLock(CLock* lock) : _lock(lock) {
    if (_lock) {
        _lock->lock();
    }
}

CAutoLock::~CAutoLock() {
    if (_lock) {
        _lock->unlock();
    }
}
