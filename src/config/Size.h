/*
 ============================================================================
 Name        : Size.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _SIZE_H_
#define _SIZE_H_

#include "Length.h"

namespace Size {

const ub4_ NAME = Length::NAME + 1;
const ub4_ PASSWORD = Length::PASSWORD + 1;
const ub4_ IP_V4 = Length::IP_V4 + 1;
const ub4_ URL = Length::URL + 1;
const ub4_ PATH = Length::PATH + 1;

// define your sizes here according to Length.h
const ub4_ SESSION_ID = Length::SESSION_ID + 1;
const ub4_ JSON = Length::JSON + 1;
}

#endif // _SIZE_H_
