/*
 ============================================================================
 Name        : CNodeGroup.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#include "CNodeGroup.h"

#include "../config/Config.h"
#include "CTrafficManager.h"
#include "CNode.h"
#include "../transaction/CTransaction.h"
#include "../common/CAutoLock.h"

CNodeGroup::CNodeGroup() :
		_worker(Config::App::THREAD_STACK_SIZE), _cond(&_mutex), _queue(
				Config::App::MESSAGE_MAX_NUM_IN_QUEUE
						* Message::MSG_FIXED_LENGTH) {
	_nodeNum = 0;
	_worker.work(this, true_v);
}

CNodeGroup::~CNodeGroup() {
}

none_ CNodeGroup::attach(CNode *node, const s1_ ip, ub2_ port,
		b4_ fd) {
	assert(node);

	if (0 == _nodeNum) {
		_ro.connect();
	}

	_nodeNum++;
	node->onAttach(this, ip, port, fd);
}

none_ CNodeGroup::detach(CNode *node) {
	assert(node);
	node->onDetach();
	_nodeNum--;

	if (0 == _nodeNum) {
		_ro.disconnect();
	}
}

bool_ CNodeGroup::putMessage(const Message::TMsg *msg) {
	assert(msg);
	CAutoLock al(&_mutex);

	log_debug("[%p %p]CNodeGroup::putMessage: write a message",
			(obj_  )msg->extra.transaction, this);

	if (!_queue.write((const ub1_ *) msg, Message::MSG_FIXED_LENGTH)) {
		log_crit(
				"[%p %p]CNodeGroup::putMessage: the size of queue is too small",
				(obj_  )msg->extra.transaction, this);

		return false_v;
	}

	_cond.unlock();

	return true_v;
}

bool_ CNodeGroup::working() {
	_mutex.lock();

	if (Message::MSG_FIXED_LENGTH > _queue.getUsedSize()) {
		_cond.lock();
	}

	ub4_ n = _queue.read((ub1_ *) &_curMsg,
			Message::MSG_FIXED_LENGTH);
	assert(Message::MSG_FIXED_LENGTH == n);

	log_debug("[%p %p]CNodeGroup::working: read a message",
			(obj_  )_curMsg.extra.transaction, this);
	_mutex.unlock();

	assert(_curMsg.extra.transaction);
	CTransaction *transaction = (CTransaction *) _curMsg.extra.transaction;
	ETransactionStatus status = transaction->getStatus();

	if (CONNECTED == status || READY == status || OVER == status) {
		if (!transaction->onMessage(&_curMsg)) {
			assert(OVER == status);
			CTrafficManager::instance()->recycleNode(transaction->getNode());
		}
	}

	return true_v;
}
