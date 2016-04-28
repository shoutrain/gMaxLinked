/*
 ============================================================================
 Name        : CBase.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : The base class
 ============================================================================
 */

#include "CBase.h"

#include <sys/time.h>
#include <sys/select.h>

log4c_category_t* CBase::_category = NULL;

bool CBase::initialize() {
	if (0 != log4c_init()) {
		return false;
	}

	_category = log4c_category_get("CHLog");

	return true;
}

void CBase::uninitialize() {
	log4c_fini();
}

log4c_category_t *CBase::getCategory() {
	return _category;
}

int CBase::sleep(unsigned int ss, unsigned int mss) {
	struct timeval delay;

	delay.tv_sec = ss;
	delay.tv_usec = mss * 1000;

	return select(0, NULL, NULL, NULL, &delay);
}

CBase::CBase() {

}

CBase::~CBase() {

}
