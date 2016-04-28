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
	unsigned int period; // seconds
	void *parameter;
	unsigned int times;

	int baseS;
	int baseUS;

	TTimer *previous;
	TTimer *next;

	int status;
};

#include <queue>

typedef std::queue<TTimer *> OperatorQueue;

// A timer manager, which can accept 1 second as the minimal unit
class CTimerManager: public CBase, public IWorkable {
public:
	CTimerManager(unsigned int maxTimerNum, unsigned int threadStackSize);
	virtual ~CTimerManager();

	// timers == 0: Infinite
	// return timer id or 0: failed
	long unsigned int setTimer(unsigned int period, void *parameter,
			unsigned int times = 1);
	void killTimer(long unsigned int timerId);

	virtual bool working();

protected:
	virtual bool onTimer(long unsigned int timerId, void *parameter) = 0;

private:
	void addTimer(TTimer *timer);
	void delTimer(TTimer *timer);

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
