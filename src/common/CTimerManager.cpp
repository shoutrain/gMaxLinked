/*
 ============================================================================
 Name        : CTimerManager.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of timer manager
 ============================================================================
 */

#include "CTimerManager.h"
#include "CAutoLock.h"

#include <sys/time.h>

CTimerManager::CTimerManager(ub4_ maxTimerNum, ub4_ threadStackSize) :
		_worker(threadStackSize), _mutex(true_v), _timerRes(maxTimerNum,
				&_mutex) {
	_timerList = null_v;
	_lastTimer = null_v;
	_curTimer = null_v;

	_worker.work(this, true_v);
}

CTimerManager::~CTimerManager() {
}

ub8_ CTimerManager::setTimer(ub4_ period, obj_ parameterI, obj_ parameterII,
		ub4_ times) {
	TTimer *timer = _timerRes.allocate();

	if (null_v == timer) {
		log_crit("CTimerManager::setTimer: no more timers can be allocated");

		return 0;
	}

	assert(NOTHING == timer->status || DELETED == timer->status);
	struct timeval curTime;
	gettimeofday(&curTime, null_v);

	timer->period = period;
	timer->parameterI = parameterI;
	timer->parameterII = parameterII;
	timer->times = times;
	timer->baseS = curTime.tv_sec;
	timer->baseUS = curTime.tv_usec;
	timer->previous = null_v;
	timer->next = null_v;
	timer->status = TO_BE_ADD;

	_mutex.lock();
	_queueForAdd.push(timer);
	_mutex.unlock();

	log_debug("CTimerManager::setTimer: time id-%p, period-%uSec, times-%u",
			timer, period, times);

	return (ub8_) timer;
}

none_ CTimerManager::killTimer(ub8_ timerId) {
	assert(timerId > 0);
	TTimer *timer = (TTimer *) timerId;
	assert(NOTHING != timer->status);
	CAutoLock al(&_mutex);

	if (TO_BE_ADD == timer->status) {
		timer->status = DELETED;
		_timerRes.reclaim(timer);

		return;
	}

	if (ADDED != timer->status) {
		return; // The timer will be deleted or has been deleted
	}

	timer->status = TO_BE_DEL;
	_queueForDel.push(timer);

	log_debug("CTimerManager::killTimer: time id-%p.", timer);
}

bool_ CTimerManager::working() {
	_mutex.lock();

	while (!_queueForAdd.empty()) {
		TTimer *timer = _queueForAdd.front();

		if (TO_BE_ADD == timer->status) {
			_addTimer(timer);
		}

		_queueForAdd.pop();
	}

	while (!_queueForDel.empty()) {
		TTimer *pTimer = _queueForDel.front();

		if (TO_BE_DEL == pTimer->status) {
			if (pTimer == _curTimer) {
				_curTimer = _curTimer->next;
			}

			_delTimer(pTimer);
		}

		_queueForDel.pop();
	}

	_mutex.unlock();

	struct timeval curTime;
	gettimeofday(&curTime, null_v);

	if (null_v == _curTimer) {
		if (null_v == _timerList) {
			// Sleep for 0.1s
			sleep(0, 100);

			return true_v;
		}

		_curTimer = _timerList;
	}

	b4_ i = _curTimer->period + _curTimer->baseS;

	if ((i < curTime.tv_sec)
			|| (i == curTime.tv_sec && _curTimer->baseUS <= curTime.tv_usec)) {
		if (__onTimer((ub8_) _curTimer, _curTimer->parameterI,
				_curTimer->parameterII)) {
			_curTimer->baseS = curTime.tv_sec;
			_curTimer->baseUS = curTime.tv_usec;

			if (0 != _curTimer->times) {
				_curTimer->times--;

				if (0 == _curTimer->times) {
					TTimer *pCurTimer = _curTimer;

					_curTimer = _curTimer->next;
					_delTimer(pCurTimer);

					return true_v;
				}
			}
		} else {
			TTimer *pCurTimer = _curTimer;

			_curTimer = _curTimer->next;
			_delTimer(pCurTimer);

			return true_v;
		}
	}

	_curTimer = _curTimer->next;

	return true_v;
}

none_ CTimerManager::_addTimer(TTimer *timer) {
	assert(null_v != timer);

	if (null_v == _timerList) {
		assert(null_v == _lastTimer);
		_lastTimer = _timerList = timer;
	} else {
		timer->previous = _lastTimer;
		_lastTimer->next = timer;
		_lastTimer = timer;
	}

	timer->status = ADDED;
}

none_ CTimerManager::_delTimer(TTimer *timer) {
	assert(null_v != timer);

	if (null_v == timer->previous) {
		assert(timer == _timerList);

		if (null_v == timer->next) {
			assert(timer == _lastTimer);
			_lastTimer = _timerList = null_v;
		} else {
			_timerList = timer->next;
			_timerList->previous = null_v;
		}
	} else {
		if (null_v == timer->next) {
			assert(timer == _lastTimer);
			_lastTimer = timer->previous;
			_lastTimer->next = null_v;
		} else {
			timer->previous->next = timer->next;
			timer->next->previous = timer->previous;
		}
	}

	timer->status = DELETED;
	_timerRes.reclaim(timer);
}
