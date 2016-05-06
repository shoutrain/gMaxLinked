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

CTransactionManager *CTransactionManager::_tm = null_v;

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

none_ CTransactionManager::work() {
	CTrafficManager::instance()->work();
}

CTransaction *CTransactionManager::findTransaction(ub8_ id) {
	assert(id);
	CAutoLock al(&_mutex);

	TransactionMap::iterator pos = _transactionMap.find(id);

	if (_transactionMap.end() == pos) {
		return null_v;
	} else {
		return pos->second;
	}
}

bool_ CTransactionManager::registerTransaction(CTransaction *transaction) {
	assert(transaction);
	CAutoLock al(&_mutex);

	ub8_ id = transaction->getId();
	TransactionMap::iterator pos = _transactionMap.find(id);

	if (_transactionMap.end() != pos) {
		return false_v;
	}

	_transactionMap.insert(TransactionMap::value_type(id, transaction));
	log_info("[%p]CTransactionManager::registerTransaction: transaction "
			"id-%lu, total registered transactions-%lu", transaction, id,
			_transactionMap.size());

	return true_v;
}

bool_ CTransactionManager::unregisterTransaction(CTransaction *transaction) {
	assert(transaction);
	CAutoLock al(&_mutex);
	ub8_ id = transaction->getId();

	if (0 == id) {
		return false_v;
	}

	TransactionMap::iterator pos = _transactionMap.find(id);

	if (_transactionMap.end() == pos) {
		return false_v;
	}

	assert(transaction == pos->second);
	_transactionMap.erase(pos);
	log_info(
			"[%p]CTransactionManager::unregisterTransaction: transaction "
			"id-%lu, total registered transactions-%lu", transaction, id,
			_transactionMap.size());

	return true_v;
}

bool_ CTransactionManager::onTimer(ub8_ timerId, obj_ parameter) {
	assert(parameter);
	CTransaction *transaction = (CTransaction *) parameter;

	log_debug("[%p]CTransactionManager::onTimer: timer id-%p", transaction,
			(obj_ )timerId);

	Message::TPDUOnTimer msg;

	msg.header.size = sizeof(Message::TPDUOnTimer);
	msg.header.type = Message::MT_CONTROL;
	msg.header.cmd = Message::MC_ON_TIMER;
	msg.header.stmp = CBase::now();
	msg.header.ver = 0x0100;
	msg.header.lang = 1;
	msg.header.seq = 0;
	msg.header.ext = (ub_8) & this;
	msg.timerId= (ub8_)timerId;

	transaction->getNode()->getGroup()->putMessage(&msg);

	return true_v;
}
