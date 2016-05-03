/*
 ============================================================================
 Name        : CTransactionManager.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#include "CTransactionManager.h"

#include "../common/CAutoLock.h"
#include "../config/Config.h"
#include "../traffic/CNode.h"
#include "../traffic/Message.h"
#include "../traffic/CTrafficManager.h"

CTransactionManager *CTransactionManager::_tm = NULL;

CTransactionManager *CTransactionManager::instance() {
	if (!_tm) {
		_tm = new CTransactionManager();
	}

	return _tm;
}

CTransactionManager::CTransactionManager() :
		CTimerManager(Config::App::TOTAL_SUPPORT_USER_NUM * 2,
				Config::App::THREAD_STACK_SIZE) {
}

CTransactionManager::~CTransactionManager() {
}

void CTransactionManager::work() {
	CTrafficManager::instance()->work();
}

CTransaction *CTransactionManager::findTransaction(unsigned int id) {
	assert(id);
	CAutoLock al(&_mutex);

	TransactionMap::iterator pos = _transactionMap.find(id);

	if (_transactionMap.end() == pos) {
		return NULL;
	} else {
		return pos->second;
	}
}

bool CTransactionManager::registerTransaction(CTransaction *transaction) {
	assert(transaction);
	CAutoLock al(&_mutex);

	unsigned int id = transaction->getId();
	TransactionMap::iterator pos = _transactionMap.find(id);

	if (_transactionMap.end() != pos) {
		return false;
	}

	_transactionMap.insert(
			TransactionMap::value_type(id, transaction));
	log_info("[%p]CTransactionManager::registerTransaction: transaction id-%u, "
			"total registered transactions-%lu", transaction, id,
			_transactionMap.size());

	return true;
}

bool CTransactionManager::unregisterTransaction(CTransaction *transaction) {
	assert(transaction);
	CAutoLock al(&_mutex);
	unsigned int id = transaction->getId();

	if (0 == id) {
		return false;
	}

	TransactionMap::iterator pos = _transactionMap.find(id);

	if (_transactionMap.end() == pos) {
		return false;
	}

	assert(transaction == pos->second);
	_transactionMap.erase(pos);
	log_info(
			"[%p]CTransactionManager::unregisterTransaction: transaction id-%u, "
					"total registered transactions-%lu", transaction,
			id, _transactionMap.size());

	return true;
}

bool CTransactionManager::onTimer(long unsigned int timerId, void *parameter) {
	assert(parameter);
	CTransaction *transaction = (CTransaction *) parameter;

	log_debug("[%p]CTransactionManager::onTimer: timer id-%p", transaction,
			(void * )timerId);

	Message::TMsg msg;
	Message::TMBTimer *body = (Message::TMBTimer *) msg.body;

	msg.header.length = sizeof(Message::TMBTimer);
	msg.header.cmd = Message::MC_TIMER;
	msg.extra.transaction = (long unsigned int) transaction;
	body->timerId = timerId;

	transaction->getNode()->getGroup()->putMessage(&msg);

	return true;
}
