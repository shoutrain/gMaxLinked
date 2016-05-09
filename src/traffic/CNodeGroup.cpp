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
				Config::App::MESSAGE_MAX_NUM_IN_QUEUE * Message::MSG_MAX_LENGTH) {
	_nodeNum = 0;
	_worker.work(this, true_v);
}

CNodeGroup::~CNodeGroup() {
}

none_ CNodeGroup::attach(CNode *node, const c1_ *ip, ub2_ port, b4_ fd) {
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

	log_debug("[%p %p]CNodeGroup::putMessage: write a message", (obj_ )msg->ext,
			this);

	if (!_queue.write((const ub1_ *) msg, msg->size)) {
		log_crit("[%p %p]CNodeGroup::putMessage: the size of queue is too "
				"small", (obj_ )msg->ext, this);

		return false_v;
	}

	_cond.unlock();

	return true_v;
}

bool_ CNodeGroup::working() {
	_mutex.lock();

	if (0 == _queue.getUsedSize()) {
		_cond.lock();
	}

	ub4_ n = _queue.read(_buffer, sizeof(ub2_));
	assert(sizeof(ub2_) == n);

	ub2_ size = (ub2_) *_buffer;

	n = _queue.read(_buffer + sizeof(ub2_), size);
	assert(size == n);

	Message::TMsg *msg = (Message::TMsg *) _buffer;
	assert(msg->ext);

	log_debug("[%p %p]CNodeGroup::working: read a message", (obj_ )msg->ext,
			this);
	_mutex.unlock();

	CTransaction *transaction = (CTransaction *) msg->ext;
	ETransactionStatus status = transaction->getStatus();

	if (CONNECTED == status || READY == status || OVER == status) {
		if (!transaction->onMessage(msg)) {
			assert(OVER == status);
			CTrafficManager::instance()->recycleNode(transaction->getNode());
		}
	}

	return true_v;
}
