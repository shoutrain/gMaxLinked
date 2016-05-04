/*
 ============================================================================
 Name        : CTransaction.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#include "CTransaction.h"
#include "CTransactionManager.h"
#include "../config/Config.h"
#include "../database/CRedisOperator.h"
#include "../traffic/CNode.h"

#include <stdlib.h>
#include <string.h>

#include "../traffic/CTrafficManager.h"

CTransaction::CTransaction(CNode *node) :
		_node(node) {
	_status = FREE;
	_id = 0;
	_keepLiveTimerId = 0;
}

CTransaction::~CTransaction() {
}

none_ CTransaction::onAttach() {
	assert(_id == 0);
	assert(_status == FREE);
	_status = CONNECTED;
	// set timer to wait the next message coming
	_keepLiveTimerId = CTransactionManager::instance()->setTimer(
			Config::App::HANDSHAKE_INTERVAL, this, 1);
}

none_ CTransaction::onDetach() {
	assert(_id == 0);
	assert(OVER == _status);
	_status = FREE;
	assert(_keepLiveTimerId == 0);
}

none_ CTransaction::over(ETransactionExitReason reason) {
	_status = OVER;

	Message::TMsg msg;
	Message::TMBOver *body = (Message::TMBOver *) &msg.body;

	msg.header.length = sizeof(Message::TMBOver);
	msg.header.cmd = Message::MC_OVER;
	msg.extra.transaction = (ub8_) this;
	body->reason = (b4_) reason;

	_node->getGroup()->putMessage(&msg);
}

bool_ CTransaction::onMessage(const Message::TMsg *msg) {
	switch (msg->header.cmd) {
	case Message::MC_HANDSHAKE:
		return onStart(msg->header.length, msg->extra.sequence,
				(Message::TMBHandshake *) msg->body);
	case Message::MC_REALTIME:
		return onRealtime(msg->extra.sequence,
				(Message::TMBRealtime *) msg->body);
	case Message::MC_TIMER:
		return onTimer((Message::TMBTimer *) msg->body);
	case Message::MC_OVER:
		return onStop((Message::TMBOver *) msg->body);
	default:
		log_info("[%p]CTransaction::onMessage: unknown message-%x, current "
				"status-%d", this, msg->header.cmd, _status);
		over(UNKNOWN_MESSAGE);

		return true_v;
	}
}

bool_ CTransaction::onStart(ub2_ length, ub4_ sequence,
		const Message::TMBHandshake *data) {
	log_debug("[%p]CTransaction::onStart: current status-%d", this, _status);

	if (CONNECTED != _status) {
		over(WRONG_STATUS);

		return true_v;
	}

	assert(_keepLiveTimerId);
	CTransactionManager::instance()->killTimer(_keepLiveTimerId);
	_keepLiveTimerId = 0;

	Message::TMsg msg;
	Message::TMBAck *ack = (Message::TMBAck *) msg.body;

	msg.header.length = sizeof(Message::TMBAck);
	msg.header.cmd = Message::MC_HANDSHAKE | Message::MS_ACK;
	msg.extra.sequence = sequence;

	if (sizeof(Message::TMBHandshake) != length
			|| Config::App::BASE_BUILD > data->build) {
		log_notice("[%p]CTransaction::onStart: client with %u or above is "
				"necessary, now it's %u", this, Config::App::BASE_BUILD,
				data->build);
		ack->result = 1;
		getNode()->send(&msg);
		over(CLIENT_TOO_OLD);

		return true_v;
	}

	if (!CTransactionManager::instance()->registerTransaction(this)) {
		_id = 0;
		log_notice("[%p]CTransaction::onStart: it's the same transtion %s "
				"on-line", this, data->serialNum);
		ack->result = 3;
		getNode()->send(&msg);
		over(SAME_TRANSACTION_ID);

		return true_v;
	}

	_node->getGroup()->ro().handleHandshake(_id, _node->getIp(),
			_node->getPort(), data);
	_status = READY;
	ack->result = 0;

	if (!getNode()->send(&msg)) {
		return true_v;
	}

	_keepLiveTimerId = CTransactionManager::instance()->setTimer(
			Config::App::HANDSHAKE_INTERVAL, this, 1);

	return true_v;
}

bool_ CTransaction::onRealtime(ub4_ sequence,
		const Message::TMBRealtime *data) {
	log_debug("[%p]CTransaction::onRealtime: current status-%d", this, _status);

	if (READY != _status) {
		over(WRONG_STATUS);

		return true_v;
	}

	assert(_keepLiveTimerId);
	CTransactionManager::instance()->killTimer(_keepLiveTimerId);
	_keepLiveTimerId = 0;
	_node->getGroup()->ro().handleEngineFields(_id, &data->ef);
	_node->getGroup()->ro().handleGeneratorFields(_id, &data->gf);

	Message::TMsg msg;
	Message::TMBAck *ack = (Message::TMBAck *) msg.body;

	msg.header.length = sizeof(Message::TMBAck);
	msg.header.cmd = Message::MC_REALTIME | Message::MS_ACK;
	msg.extra.sequence = sequence;
	ack->result = 0;

	if (!getNode()->send(&msg)) {
		return true_v;
	}

	_keepLiveTimerId = CTransactionManager::instance()->setTimer(
			Config::App::HANDSHAKE_INTERVAL, this, 1);

	return true_v;
}

bool_ CTransaction::onTimer(const Message::TMBTimer *data) {
	if (_keepLiveTimerId == data->timerId) {
		log_debug("[%p]CTransaction::onTimer: current status-%d", this,
				_status);
		over(TIME_OUT);
	} else {
		// invalid time id
	}

	return true_v;
}

bool_ CTransaction::onStop(const Message::TMBOver *data) {
	log_debug("[%p]CTransaction::onStop: current status-%d", this, _status);
	_status = OVER;

	switch (data->reason) {
	case WRONG_STATUS:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"wrong status");
		break;
	case CLIENT_TOO_OLD:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"client too old");
		break;
	case SAME_TRANSACTION_ID:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"same owner id");
		break;
	case TIME_OUT:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"time out");
		break;
	case UNKNOWN_MESSAGE:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"unknown message");
		break;
	case CONNECTION_BROKEN:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"connection broken");
		break;
	case CONNECTION_ERROR:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"connection error");
		break;
	case CANNOT_RECV_DATA:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"cannot recv data");
		break;
	default:
		assert(0);
	}

	if (_keepLiveTimerId) {
		CTransactionManager::instance()->killTimer(_keepLiveTimerId);
		_keepLiveTimerId = 0;
	}

	if (_id) {
		CTransactionManager::instance()->unregisterTransaction(this);
		_node->getGroup()->ro().handleOffline(_id);
		_id = 0;
	}

	return false_v;
}

