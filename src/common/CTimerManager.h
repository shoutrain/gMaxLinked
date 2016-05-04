/*
 ============================================================================
 Name        : CTimerManager.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of timer manager
 ============================================================================
 */

#ifndef _C_TIMERMANAGER_H_
#define _C_TIMERMANAGER_H_

#include "CBase.h"
#include "IWorkable.h"
#include "CWorker.h"
#include "CMutex.h"
#include "CResource.h"

enum ETimerStatus {
	NOTHING = 0x0000,

	TO_BE_ADD = 0x0001, ADDED = 0x0002, TO_BE_DEL = 0x0004, DELETED = 0x0008,
};

struct TTimer {
	ub4_ period; // seconds
	obj_ parameter;
	ub4_ times;

	b4_ baseS;
	b4_ baseUS;

	TTimer *previous;
	TTimer *next;

	b4_ status;
};

#include <queue>

typedef std::queue<TTimer *> OperatorQueue;

// A timer manager, which can accept 1 second as the minimal unit
class CTimerManager: public CBase, public IWorkable {
public:
	CTimerManager(ub4_ maxTimerNum, ub4_ threadStackSize);
	virtual ~CTimerManager();

	// timers == 0: Infinite
	// return timer id or 0: failed
	ub8_ setTimer(ub4_ period, obj_ parameter,	ub4_ times = 1);
	none_ killTimer(ub8_ timerId);

	virtual bool_ working();

protected:
	virtual bool_ onTimer(ub8_ timerId, obj_ parameter) = 0;

private:
	none_ addTimer(TTimer *timer);
	none_ delTimer(TTimer *timer);

	CWorker _worker;
	CMutex _mutex;

	CResource<TTimer, CTimerManager> _timerRes;
	TTimer *_timerList;
	TTimer *_lastTimer;

	OperatorQueue _queueForAdd;
	OperatorQueue _queueForDel;
	TTimer *_curTimer;
};

#endif // _C_TIMER_MANAGER_H_
