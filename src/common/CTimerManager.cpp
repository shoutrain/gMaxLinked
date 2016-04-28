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

CTimerManager::CTimerManager(unsigned int maxTimerNum,
		unsigned int threadStackSize) :
		_worker(threadStackSize), _mutex(true), _timerRes(maxTimerNum, &_mutex) {
	_timerList = NULL;
	_lastTimer = NULL;
	_curTimer = NULL;

	_worker.work(this, true);
}

CTimerManager::~CTimerManager() {
}

long unsigned int CTimerManager::setTimer(unsigned int period, void *parameter,
		unsigned int times) {
	TTimer *timer = _timerRes.allocate();

	if (NULL == timer) {
		log_crit("CTimerManager::setTimer: no more timers can be allocated");

		return 0;
	}

	assert(NOTHING == timer->status || DELETED == timer->status);
	struct timeval curTime;
	gettimeofday(&curTime, NULL);

	timer->period = period;
	timer->parameter = parameter;
	timer->times = times;
	timer->baseS = curTime.tv_sec;
	timer->baseUS = curTime.tv_usec;
	timer->previous = NULL;
	timer->next = NULL;
	timer->status = TO_BE_ADD;

	_mutex.lock();
	_queueForAdd.push(timer);
	_mutex.unlock();

	log_debug(
			"CTimerManager::setTimer: from-%p, time id-%p, period-%uSec, times-%u",
			parameter, timer, period, times);

	return (long unsigned int) timer;
}

void CTimerManager::killTimer(long unsigned int timerId) {
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

bool CTimerManager::working() {
	_mutex.lock();

	while (!_queueForAdd.empty()) {
		TTimer *timer = _queueForAdd.front();

		if (TO_BE_ADD == timer->status) {
			addTimer(timer);
		}

		_queueForAdd.pop();
	}

	while (!_queueForDel.empty()) {
		TTimer *pTimer = _queueForDel.front();

		if (TO_BE_DEL == pTimer->status) {
			if (pTimer == _curTimer) {
				_curTimer = _curTimer->next;
			}

			delTimer(pTimer);
		}

		_queueForDel.pop();
	}

	_mutex.unlock();

	struct timeval curTime;
	gettimeofday(&curTime, NULL);

	if (NULL == _curTimer) {
		if (NULL == _timerList) {
			// Sleep for 0.1s
			sleep(0, 100);

			return true;
		}

		_curTimer = _timerList;
	}

	int i = _curTimer->period + _curTimer->baseS;

	if ((i < curTime.tv_sec)
			|| (i == curTime.tv_sec && _curTimer->baseUS <= curTime.tv_usec)) {
		if (onTimer((long unsigned int) _curTimer, _curTimer->parameter)) {
			_curTimer->baseS = curTime.tv_sec;
			_curTimer->baseUS = curTime.tv_usec;

			if (0 != _curTimer->times) {
				_curTimer->times--;

				if (0 == _curTimer->times) {
					TTimer *pCurTimer = _curTimer;

					_curTimer = _curTimer->next;
					delTimer(pCurTimer);

					return true;
				}
			}
		} else {
			TTimer *pCurTimer = _curTimer;

			_curTimer = _curTimer->next;
			delTimer(pCurTimer);

			return true;
		}
	}

	_curTimer = _curTimer->next;

	return true;
}

void CTimerManager::addTimer(TTimer *timer) {
	assert(NULL != timer);

	if (NULL == _timerList) {
		assert(NULL == _lastTimer);
		_lastTimer = _timerList = timer;
	} else {
		timer->previous = _lastTimer;
		_lastTimer->next = timer;
		_lastTimer = timer;
	}

	timer->status = ADDED;
}

void CTimerManager::delTimer(TTimer *timer) {
	assert(NULL != timer);

	if (NULL == timer->previous) {
		assert(timer == _timerList);

		if (NULL == timer->next) {
			assert(timer == _lastTimer);
			_lastTimer = _timerList = NULL;
		} else {
			_timerList = timer->next;
			_timerList->previous = NULL;
		}
	} else {
		if (NULL == timer->next) {
			assert(timer == _lastTimer);
			_lastTimer = timer->previous;
			_lastTimer->next = NULL;
		} else {
			timer->previous->next = timer->next;
			timer->next->previous = timer->previous;
		}
	}

	timer->status = DELETED;
	_timerRes.reclaim(timer);
}
