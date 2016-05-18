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
	memset(_sessionId, 0, Size::SESSION_ID);
	_id = 0;
	_keepLiveTimerId = 0;
	_heartbeat = false_v;
	_seqCounter = 0;
}

CTransaction::~CTransaction() {
}

none_ CTransaction::onAttach() {
	assert(0 == _sessionId[0]);
	assert(0 == _id);
	assert(FREE == _status);
	assert(0 == _keepLiveTimerId);
	assert(false_v == _heartbeat);
	assert(0 == _seqCounter);
	assert(0 == _mapSeq2Timer.size());

	_status = CONNECTED;
	// set timer to wait the next message coming
	_keepLiveTimerId = CTransactionManager::instance()->setTimer(
			Config::App::HEARTBEAT_INTERVAL, this, (obj_) _status, 1);
}

none_ CTransaction::onDetach() {
	assert(0 == _sessionId[0]);
	assert(0 == _id);
	assert(OVER == _status);
	assert(0 == _keepLiveTimerId);
	assert(false_v == _heartbeat);
	assert(0 == _seqCounter);
	assert(0 == _mapSeq2Timer.size());

	_status = FREE;
}

none_ CTransaction::over(ETransactionExitReason reason) {
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

	__send((Message::TMsg *) &msg, false_v, true_v);
}

bool_ CTransaction::onMessage(const Message::TMsg *msg) {
	if (Message::MT_CONTROL == msg->type
			&& Message::MC_HAND_SHAKE == msg->cmd) {
		return __onStart((Message::TPDUHandShake *) msg);
	} else if (Message::MT_CONTROL | Message::MT_SIGN_ACK == msg->type
			&& Message::MC_HEART_BEAT == msg->cmd) {
		return __onHeartBeat((Message::TPDUHeartBeat *) msg);
	} else if (Message::MC_ON_TIMER == msg->cmd) {
		return __onTimer((Message::TPDUOnTimer *) msg);
	} else if (Message::MC_ON_OVER == msg->cmd) {
		return __onStop((Message::TPUDOnOver *) msg);
	} else {
		log_info("[%p]CTransaction::onMessage: unknown message-%x-%x, current "
				"status-%d", this, msg->type, msg->cmd, _status);
		over(UNKNOWN_MESSAGE);

		return true_v;
	}
}

bool_ CTransaction::__onStart(const Message::TPDUHandShake* msg) {
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
		log_notice("[%p]CTransaction::onStart: client version with %u or above"
				" is necessary, now it's %u", this, Config::App::BASE_BUILD,
				msg->build);
		msgAck.ack.code = CLIENT_TOO_OLD;
		__send((Message::TMsg *) &msgAck, false_v);
		over(CLIENT_TOO_OLD);

		return true_v;
	}

	memcpy(_sessionId, msg->sessionId, Size::SESSION_ID);
	_id = _node->getGroup()->ro().verifyHandshake(_sessionId);

	if (0 == _id) {
		log_notice("[%p]CTransaction::onStart: cannot find corresponding "
				"sessionId-%s", this, _sessionId);
		memset(_sessionId, 0, Size::SESSION_ID);
		_id = 0;
		msgAck.ack.code = NO_THE_SESSION_FOUND;
		__send((Message::TMsg *) &msgAck, false_v);
		over(NO_THE_SESSION_FOUND);

		return true_v;
	}

	if (!CTransactionManager::instance()->registerTransaction(this)) {
		log_notice("[%p]CTransaction::onStart: there is an transaction with "
				"the same sessionId-%s and id-%lu", this, _sessionId, _id);
		memset(_sessionId, 0, Size::SESSION_ID);
		_id = 0;
		msgAck.ack.code = SAME_SESSION_ID;
		__send((Message::TMsg *) &msgAck, false_v);
		over(SAME_SESSION_ID);

		return true_v;
	}

	_status = READY;
	msgAck.ack.code = 0;

	if (true_v == __send((Message::TMsg *) &msgAck, false_v)) {
		_heartbeat = true_v;
		_keepLiveTimerId = CTransactionManager::instance()->setTimer(
				Config::App::HEARTBEAT_INTERVAL, this, (obj_) _status, 0);
	}

	return true_v;
}

bool_ CTransaction::__onHeartBeat(const Message::TPDUHeartBeat *msg) {
	log_debug("[%p]CTransaction::onHeartBeat: current status-%d", this,
			_status);

	if (READY != _status) {
		over(WRONG_STATUS);

		return true_v;
	}

	_heartbeat = true_v;

	return true_v;
}

bool_ CTransaction::__onTimer(const Message::TPDUOnTimer *msg) {
	if (_keepLiveTimerId == msg->timerId) {
		if (_status != msg->parameter) {
			log_debug("[%p]CTransaction::onTimer: timer status(%d) != "
					"current(%d) status", this, msg->parameter, _status);

			return true_v;
		}

		if (CONNECTED == _status) {
			log_debug("[%p]CTransaction::onTimer: waiting handshake timeout, "
					"current status-%d", this, _status);
			over(TIME_OUT);
		} else if (READY == _status) {
			if (false_v == _heartbeat) {
				log_debug("[%p]CTransaction::onTimer: heartbeat timeout, "
						"current status-%d", this, _status);
				over(TIME_OUT);
			} else {
				Message::TPDUHeartBeat message;

				message.header.size = sizeof(Message::TPDUHeartBeat);
				message.header.type = Message::MT_CONTROL;
				message.header.cmd = Message::MC_HEART_BEAT;
				message.header.ver = 0x0100;
				message.header.lang = 1;
				message.header.seq = 0;
				message.header.stmp = CBase::now();
				message.header.ext = 0;

				__send((Message::TMsg *) &message, false_v);
				_heartbeat = false_v;
			}
		} else {
			assert(false_v);
		}

	} else {
		if (READY != _status) {
			log_debug("[%p]CTransaction::onTimer: seq(%u)'s status(%d) is "
					"expected, but now it's status(%d)", this, msg->parameter,
					READY, _status);

			return true_v;
		}

		ub4_ seq = msg->parameter;
		MapSeq2Timer::iterator pos = _mapSeq2Timer.find(msg->timerId);

		if (_mapSeq2Timer.end() != pos) {
			log_debug("[%p]CTransaction::onTimer: waitting ack timeout, "
					"current status-%d, seq-%u", this, _status, seq);
			over(TIME_OUT);
		} else {
			assert(false_v);
		}
	}

	return true_v;
}

bool_ CTransaction::__onStop(const Message::TPUDOnOver *msg) {
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
	case NO_THE_SESSION_FOUND:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"no the session found");
		break;
	case SAME_SESSION_ID:
		log_debug("[%p]CTransaction::onStop: reason-%s", this,
				"same session id");
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

	for (MapSeq2Timer::iterator pos = _mapSeq2Timer.begin();
			pos != _mapSeq2Timer.end(); pos++) {
		CTransactionManager::instance()->killTimer(pos->second);
	}

	_mapSeq2Timer.clear();

	if (0 != _sessionId[0]) {
		CTransactionManager::instance()->unregisterTransaction(this);
		memset(_sessionId, 0, Size::SESSION_ID);
		_id = 0;
	}

	_heartbeat = false_v;
	_seqCounter = 0;

	return false_v;
}

bool_ CTransaction::__send(Message::TMsg *msg, bool_ waitAck, bool_ interval) {
	assert(null_v != msg);

	if (false_v == interval) {
		if (true_v == waitAck) {
			ub4_ seq = ++_seqCounter;

			if (0 == seq) {
				seq = ++_seqCounter;
			}

			msg->seq = seq;
		}

		bool_ ret = _node->send(msg);

		if (true_v == ret && true_v == waitAck) {
			ub8_ timeId = CTransactionManager::instance()->setTimer(
					Config::App::HEARTBEAT_INTERVAL, this,
					(obj_) (ub8_) msg->seq, 1);

			_mapSeq2Timer.insert(MapSeq2Timer::value_type(msg->seq, timeId));
		}

		return ret;
	} else {
		return _node->getGroup()->putMessage(msg);
	}
}
