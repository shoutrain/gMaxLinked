/*
 ============================================================================
 Name        : CWorkable.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of pthread
 ============================================================================
 */

#ifndef _C_WORKER_H_
#define _C_WORKER_H_

#include "CBase.h"
#include "CMutex.h"
#include "CCond.h"

class IWorkable;

class CWorker: public CBase {
public:
	CWorker(unsigned int threadStackSize /* k */);
	virtual ~CWorker();

	// sync is invalid unless bInformed = true
	void work(IWorkable *workable, bool informed = false, bool sync = false);

	static void stop() {
		_workingCondition = false;
	}

	static unsigned int getTotalNum() {
		return _workerNum;
	}

private:
	static void *run(void *object);

	static CMutex _mutexWorker;
	static unsigned int _workerNum;
	static bool _workingCondition;

	bool createThread();

	unsigned int _threadStackSize; // k
	pthread_t _handle;
	bool _informed;
	CMutex _mutexInformed;
	CCond _condInformed;

	IWorkable *_workable;
};

#endif // _C_WORKER_H_
