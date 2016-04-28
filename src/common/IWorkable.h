/*
 ============================================================================
 Name        : IWorkable.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : The protocol of CWorker, used by CWorker instances
 ============================================================================
 */

#ifndef _I_WORKABLE_H_
#define _I_WORKABLE_H_

class IWorkable {
public:

	virtual ~IWorkable() {
	} // to avoid waring
	virtual bool working() = 0;
};

#endif // _I_WORKABLE_H_
