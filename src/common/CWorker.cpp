/*
 ============================================================================
 Name        : CWorkable.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of pthread
 ============================================================================
 */

#include "CWorker.h"
#include "CAutoLock.h"
#include "IWorkable.h"

CMutex CWorker::_mutexWorker;
ub4_ CWorker::_workerNum = 0;
bool_ CWorker::_workingCondition = true_v;

CWorker::CWorker(ub4_ threadStackSize) :
		_condInformed(&_mutexInformed) {
	_threadStackSize = threadStackSize * 1024;
	_handle = 0;
	_informed = false_v;
	_workable = null_v;
}

CWorker::~CWorker() {
}

none_ CWorker::work(IWorkable *workable, bool_ informed, bool_ sync) {
	if (_handle) {
		return;
	}

	assert(null_v != workable);
	_workable = workable;

	if (!informed) {
		createThread();
	} else {
		CAutoLock al(&_mutexInformed);

		_informed = informed;

		if (false_v == createThread()) {
			return;
		}

		if (_informed) {
			_condInformed.lock();
		}

		if (sync) {
			pthread_join(_handle, null_v);
		}
	}
}

obj_ CWorker::run(obj_ object) {
	CWorker *worker = (CWorker *) object;

	_mutexWorker.lock();
	_workerNum++;
	log_debug("Worker(%016lu) started, totally %d workers ", pthread_self(),
			_workerNum);
	_mutexWorker.unlock();

	worker->_mutexInformed.lock();

	if (worker->_informed) {
		worker->_informed = false_v;
		worker->_condInformed.unlock();
	}

	worker->_mutexInformed.unlock();

	while (_workingCondition) {
		if (!worker->_workable->working()) {
			break;
		}
	}

	_mutexWorker.lock();
	_workerNum--;
	log_debug("Worker(%016lu) ended, totally %d workers", pthread_self(),
			_workerNum);
	_mutexWorker.unlock();

	worker->_handle = 0;

	return null_v;
}

bool_ CWorker::createThread() {
	pthread_attr_t attr;

	if (0 != pthread_attr_init(&attr)) {
		log_fatal("CWorker::createThread: failed to call pthread_attr_init");

		return false_v;
	}

	if (0 != pthread_attr_setstacksize(&attr, _threadStackSize)) {
		log_fatal(
				"CWorker::createThread: failed to call "
				"pthread_attr_setstacksize");

		return false_v;
	}

	if (0 != pthread_create(&_handle, &attr, CWorker::run, (obj_ ) this)) {
		log_fatal("CWorker::createThread: failed to call pthread_create");

		return false_v;
	}

	pthread_attr_destroy(&attr);

	return true_v;

}
