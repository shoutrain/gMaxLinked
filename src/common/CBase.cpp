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

	_category = log4c_category_get("HFRSLog");

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

ub8_ CBase::now() {
	timeval tv;

	if (0 != gettimeofday(&tv, null_v)) {
		log_fatal("CBase::now: failed to call gettimeofday");

		return 0;
	}

	return tv.tv_sec * 1000000 + tv.tv_usec;
}

CBase::CBase() {

}

CBase::~CBase() {

}
