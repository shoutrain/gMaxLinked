/*
 ============================================================================
 Name        : CLock
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : The protocol of lockers
 ============================================================================
 */

#ifndef _C_LOCK_H_
#define _C_LOCK_H_

#include "CBase.h"

class CLock : public CBase {
public:
    virtual bool_ lock(bool_ check = false_v) = 0;
    virtual none_ unlock() = 0;

protected:
    CLock() {}

    virtual ~CLock() {}
};

#endif  // _C_LOCK_H_
