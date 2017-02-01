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

CNodeGroup::CNodeGroup() :
		_worker(Config::App::THREAD_STACK_SIZE), _queue(
				Config::App::MESSAGE_MAX_NUM_IN_QUEUE * Message::MSG_MAX_LENGTH,
				&_mutex) {
	_pos = _container.end();
	_worker.work(this, true_v);
}

CNodeGroup::~CNodeGroup() {
}

bool_ CNodeGroup::attach(CNode *node, const c1_ *ip, ub2_ port, b4_ fd) {
	assert(node);
	CAutoLock al(&_mutex);

	if (false_v == node->onAttach(this, ip, port, fd)) {
		return false_v;
	}

	if (0 == _container.size()) {
		_ro.connect();
	}

	_container.push_back(node);

	return true_v;
}

none_ CNodeGroup::detach(CNode *node) {
	assert(node);
	CAutoLock al(&_mutex);

	if (*_pos == node) {
		_pos++;
	}

	_container.remove(node);

	if (0 == _container.size()) {
		_ro.disconnect();
	}

	node->onDetach();
}

bool_ CNodeGroup::putMessage(const Message::TMsg *msg) {
	assert(msg);
	log_debug("[%p %p]CNodeGroup::putMessage: write a message", (obj_ )msg->ext,
			this);

	if (!_queue.write((const ub1_ *) msg, msg->size)) {
		log_crit("[%p %p]CNodeGroup::putMessage: the size of queue is too "
				"small", (obj_ )msg->ext, this);

		return false_v;
	}

	return true_v;
}

bool_ CNodeGroup::working() {
	bool_ rollingQueue = _rollingQueue();
	bool_ rollingNode = _rollingNode();

	if (false_v == rollingQueue && false_v == rollingNode) {
		sleep(0, 50);
	}

	return true_v;
}

bool_ CNodeGroup::_rollingQueue() {
	if (0 == _queue.getUsedSize()) {
		return false_v;
	}

	ub4_ n = _queue.read(_buffer, sizeof(ub2_));
	assert(sizeof(ub2_) == n);

	ub2_ size = *(ub2_ *) _buffer;

	n = _queue.read(_buffer + sizeof(ub2_), size - sizeof(ub2_));

	if (n + sizeof(ub2_) != size) {
		log_error("[%p]CNodeGroup::rollingQueue: size doesn't match", this);

		return false_v;
	}

	Message::TMsg *msg = (Message::TMsg *) _buffer;
	assert(msg->ext);

	log_debug("[%p %p]CNodeGroup::rollingQueue: read a message",
			(obj_ )msg->ext, this);

	CTransaction *transaction = (CTransaction *) msg->ext;
	ETransactionStatus status = transaction->getStatus();

	if (ETransactionStatus::CONNECTED == status
			|| ETransactionStatus::READY == status) {
		if (!transaction->onMessage(msg)) {
			assert(ETransactionStatus::OVER == transaction->getStatus());
			CTrafficManager::instance()->recycleNode(transaction->getNode());
		}
	}

	return true_v;
}

bool_ CNodeGroup::_rollingNode() {
	_mutex.lock();

	if (0 == _container.size()) {
		_mutex.unlock();

		return false_v;
	}

	if (_container.end() == _pos) {
		_pos = _container.begin();
	}

	CNode *node = *_pos++;

	_mutex.unlock();

	CTransaction *transaction = node->getTransaction();
	ETransactionStatus status = transaction->getStatus();

	if (ETransactionStatus::READY == status) {
		transaction->onCheck();
	}

	return true_v;
}
