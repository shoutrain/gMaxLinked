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
	CWorker(ub4_ threadStackSize /* k */);
	virtual ~CWorker();

	// sync is invalid unless bInformed = true_v
	none_ work(IWorkable *workable, bool_ informed = false_v,
			bool_ sync = false_v);

	static none_ stop() {
		_workingCondition = false_v;
	}

	static ub4_ getTotalNum() {
		return _workerNum;
	}

private:
	static obj_ _run(obj_ object);

	static CMutex _mutexWorker;
	static ub4_ _workerNum;
	static bool_ _workingCondition;

	bool_ createThread();

	ub4_ _threadStackSize; // k
	pthread_t _handle;
	bool_ _informed;
	CMutex _mutexInformed;
	CCond _condInformed;

	IWorkable *_workable;
};

#endif // _C_WORKER_H_
