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

log4c_category_t* CBase::_category = null_v;

bool_ CBase::initialize() {
	if (0 != log4c_init()) {
		return false_v;
	}

	_category = log4c_category_get("CHLog");

	return true_v;
}

none_ CBase::uninitialize() {
	log4c_fini();
}

log4c_category_t *CBase::getCategory() {
	return _category;
}

b4_ CBase::sleep(ub4_ ss, ub4_ mss) {
	struct timeval delay;

	delay.tv_sec = ss;
	delay.tv_usec = mss * 1000;

	return select(0, null_v, null_v, null_v, &delay);
}

CBase::CBase() {

}

CBase::~CBase() {

}
