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
			Config::App::HEARTBEAT_INTERVAL, this, 1);
}

none_ CTransaction::onDetach() {
	assert(_id == 0);
	assert(OVER == _status);
	_status = FREE;
	assert(_keepLiveTimerId == 0);
}

none_ CTransaction::over(ETransactionExitReason reason) {
	_status = OVER;

	Message::TPUDOnOver msg;

	msg.header.size = sizeof(Message::TPUDOnOver);
	msg.header.type = Message::MT_CONTROL;
	msg.header.cmd = Message::MC_ON_OVER;
	msg.header.stmp = CBase::now();
	msg.header.ver = 0x0100;
	msg.header.lang = 1;
	msg.header.seq = 0;
	msg.header.ext = (ub8_) this;
	msg.reason = (b4_) reason;

	_node->getGroup()->putMessage((Message::TMsg *)&msg);
}

bool_ CTransaction::onMessage(const Message::TMsg *msg) {
	switch (msg->cmd) {
	case Message::MC_HAND_SHAKE:
		return onStart((Message::TPDUHandShake *) msg);
	case Message::MC_HEART_BEAT:
		return onHeartBeat((Message::TPDUHeartBeat *) msg);
	case Message::MC_ON_TIMER:
		return onTimer((Message::TPDUOnTimer *) msg);
	case Message::MC_ON_OVER:
		return onStop((Message::TPUDOnOver *) msg);
	default:
		log_info("[%p]CTransaction::onMessage: unknown message-%x, current "
				"status-%d", this, msg->cmd, _status);
		over(UNKNOWN_MESSAGE);

		return true_v;
	}
}

bool_ CTransaction::onStart(const Message::TPDUHandShake* msg) {
	log_debug("[%p]CTransaction::onStart: current status-%d", this, _status);

	if (CONNECTED != _status) {
		over(WRONG_STATUS);

		return true_v;
	}

	assert(_keepLiveTimerId);
	CTransactionManager::instance()->killTimer(_keepLiveTimerId);
	_keepLiveTimerId = 0;

	Message::TPDUHandShakeAck msgAck;

	memcpy(&msgAck, msg, sizeof(Message::THeader));
	msgAck.header.size = sizeof(Message::TPDUHandShakeAck);
	msgAck.header.type |= Message::MT_SIGN_ACK;
	msgAck.header.stmp = CBase::now();
	msgAck.ack.code = 0;

	if (Config::App::BASE_BUILD > msg->build) {
		log_notice("[%p]CTransaction::onStart: client version with %u or above "
				" is necessary, now it's %u", this, Config::App::BASE_BUILD,
				msg->build);
		msgAck.ack.code = CLIENT_TOO_OLD;
		getNode()->send((Message::TMsg *)&msgAck);
		over(CLIENT_TOO_OLD);

		return true_v;
	}

	_id = msg->id;

	if (!CTransactionManager::instance()->registerTransaction(this)) {
		log_notice("[%p]CTransaction::onStart: there is a on-line transaction "
				"with id of %lu", this, _id);
		_id = 0;
		msgAck.ack.code = SAME_TRANSACTION_ID;
		getNode()->send((Message::TMsg *)&msgAck);
		over(SAME_TRANSACTION_ID);

		return true_v;
	}

	_status = READY;
	msgAck.ack.code = 0;

	if (!getNode()->send((Message::TMsg *)&msgAck)) {
		return true_v;
	}

	_keepLiveTimerId = CTransactionManager::instance()->setTimer(
			Config::App::HEARTBEAT_INTERVAL, this, 1);

	return true_v;
}

bool_ CTransaction::onHeartBeat(const Message::TPDUHeartBeat *msg) {
	log_debug("[%p]CTransaction::onHeartBeat: current status-%d", this, _status);

	if (READY != _status) {
		over(WRONG_STATUS);

		return true_v;
	}

	assert(_keepLiveTimerId);
	CTransactionManager::instance()->killTimer(_keepLiveTimerId);
	_keepLiveTimerId = 0;

	Message::TPDUHeartBeatAck msgAck;

	memcpy(&msgAck, msg, sizeof(Message::THeader));
	msgAck.header.size = sizeof(Message::TPDUHeartBeatAck);
	msgAck.header.type |= Message::MT_SIGN_ACK;
	msgAck.header.stmp = CBase::now();
	msgAck.ack.code = 0;

	if (!getNode()->send((Message::TMsg *)&msgAck)) {
		return true_v;
	}

	_keepLiveTimerId = CTransactionManager::instance()->setTimer(
			Config::App::HEARTBEAT_INTERVAL, this, 1);

	return true_v;
}

bool_ CTransaction::onTimer(const Message::TPDUOnTimer *msg) {
	if (_keepLiveTimerId == msg->timerId) {
		log_debug("[%p]CTransaction::onTimer: current status-%d", this,
				_status);
		over(TIME_OUT);
	} else {
		assert(false_v);
		// invalid time id and do nothing
	}

	return true_v;
}

bool_ CTransaction::onStop(const Message::TPUDOnOver *msg) {
	log_debug("[%p]CTransaction::onStop: current status-%d", this, _status);
	_status = OVER;

	switch (msg->reason) {
	case WRONG_STATUS:
		log_debug("[%p]CTransaction::onStop: reason-%s", this, "wrong status");
		break;
	case CLIENT_TOO_OLD:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"client too old");
		break;
	case SAME_TRANSACTION_ID:
		log_debug("[%p]CTransaction::onStop: reason-%s", this, "same owner id");
		break;
	case TIME_OUT:
		log_debug("[%p]CTransaction::onStop: reason-%s", this, "time out");
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
		assert(false_v);
	}

	if (_keepLiveTimerId) {
		CTransactionManager::instance()->killTimer(_keepLiveTimerId);
		_keepLiveTimerId = 0;
	}

	if (_id) {
		CTransactionManager::instance()->unregisterTransaction(this);
		_id = 0;
	}

	return false_v;
}

