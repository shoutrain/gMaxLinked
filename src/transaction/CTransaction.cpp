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

#include <stdlib.h>
#include <string.h>

#include "../config/Config.h"
#include "../database/CRedisOperator.h"
#include "../traffic/CNode.h"
#include "../traffic/CTrafficManager.h"
#include "CTransactionManager.h"

CTransaction::CTransaction(CNode *node) : _node(node) {
    _status = ETransactionStatus::FREE;
    _build = 0;
    _lastUpdate = 0;
    memset(_sessionId, 0, Size::SESSION_ID);
    _id = 0;
    _keepLiveTimerId = 0;
    _heartbeat = false_v;
    _seqCounter = 0;
}

CTransaction::~CTransaction() {}

bool_ CTransaction::onAttach() {
    assert(0 == _build);
    assert(0 == _lastUpdate);
    assert(0 == _sessionId[0]);
    assert(0 == _id);
    assert(ETransactionStatus::FREE == _status);
    assert(0 == _keepLiveTimerId);
    assert(false_v == _heartbeat);
    assert(0 == _seqCounter);
    assert(0 == _mapSeq2Timer.size());

    _status = ETransactionStatus::CONNECTED;

    // set timer to wait the next message coming
    _keepLiveTimerId = CTransactionManager::instance()->setTimer(
        Config::App::HEARTBEAT_INTERVAL, this, (obj_)_status, 1);

    if (0 == _keepLiveTimerId) {
        return false_v;
    }

    return true_v;
}

none_ CTransaction::onDetach() {
    assert(0 == _sessionId[0]);
    assert(0 == _id);
    assert(ETransactionStatus::OVER == _status);
    assert(0 == _keepLiveTimerId);
    assert(false_v == _heartbeat);
    assert(0 == _seqCounter);
    assert(0 == _mapSeq2Timer.size());

    _status = ETransactionStatus::FREE;
}

bool_ CTransaction::over(ETransactionExitReason reason, bool_ useQueue) {
    Message::TPDUOnOver msg;

    msg.header.size = sizeof(Message::TPDUOnOver);
    msg.header.type = Message::MT_CONTROL;
    msg.header.cmd = Message::MC_ON_OVER;
    msg.header.ver = Config::App::PROTOCOL_VERSION;
    msg.header.lang = Message::ML_CN;
    msg.header.seq = 0;
    msg.header.stmp = CBase::now();
    msg.header.ext = (ub8_)this;
    msg.reason = (b4_)reason;

    if (true_v == useQueue) {
        return _node->getGroup()->putMessage((const Message::TMsg *)&msg);
    } else {
        return __onStop(&msg);
    }
}

bool_ CTransaction::onMessage(const Message::TMsg *msg) {
    if (Message::MT_ACCOUNT == msg->type &&
        Message::MC_HAND_SHAKE == msg->cmd) {
        return __onStart((Message::TPDUHandShake *)msg);
    } else if ((Message::MT_CONTROL | Message::MT_SIGN_ACK) == msg->type &&
               Message::MC_HEART_BEAT == msg->cmd) {
        return __onHeartBeat((Message::TPDUHeartBeatAck *)msg);
    } else if (Message::MT_SERVICE == msg->type &&
               Message::MC_SEND_MSG == msg->cmd) {
        return __onSendMsg((Message::TPDUSendMsg *)msg);
    } else if ((Message::MT_SERVICE | Message::MT_SIGN_ACK) == msg->type &&
               Message::MC_PUSH_MSG == msg->cmd) {
        return __onPushMsg((Message::TPDUPushMsgAck *)msg);
    } else if (Message::MC_ON_TIMER == msg->cmd) {
        return __onTimer((Message::TPDUOnTimer *)msg);
    } else if (Message::MC_ON_OVER == msg->cmd) {
        return __onStop((Message::TPDUOnOver *)msg);
    } else {
        log_info(
            "[%p]CTransaction::onMessage: unknown message-%x-%x, current "
            "status-%d",
            this, msg->type, msg->cmd, (int)_status);
        return over(ETransactionExitReason::UNKNOWN_MESSAGE);
    }
}

void CTransaction::onCheck() { _node->getGroup()->ro().checkMessages(this); }

bool_ CTransaction::__onStart(const Message::TPDUHandShake *msg) {
    log_debug("[%p]CTransaction::onStart: current status-%d", this,
              (int)_status);

    if (ETransactionStatus::CONNECTED != _status) {
        return over(ETransactionExitReason::WRONG_STATUS);
    }

    assert(_keepLiveTimerId);
    CTransactionManager::instance()->killTimer(_keepLiveTimerId);
    _keepLiveTimerId = 0;

    Message::TPDUHandShakeAck msgAck;

    memcpy(&msgAck, msg, sizeof(Message::THeader));
    msgAck.header.size = sizeof(Message::TPDUHandShakeAck);
    msgAck.header.type |= Message::MT_SIGN_ACK;
    msgAck.ack.code = 0;

    if (Config::App::BASE_BUILD > msg->build) {
        log_notice(
            "[%p]CTransaction::onStart: client version with %u or above"
            " is necessary, now it's %u",
            this, Config::App::BASE_BUILD, msg->build);
        msgAck.ack.code = (ub2_)ETransactionExitReason::CLIENT_TOO_OLD;
        __send((Message::TMsg *)&msgAck, false_v);

        return over(ETransactionExitReason::CLIENT_TOO_OLD);
    }

    memcpy(_sessionId, msg->sessionId, Size::SESSION_ID);
    _id = _node->getGroup()->ro().verifyHandshake(_sessionId);

    if (0 == _id) {
        log_notice(
            "[%p]CTransaction::onStart: cannot find corresponding "
            "sessionId-%s",
            this, _sessionId);
        memset(_sessionId, 0, Size::SESSION_ID);
        _id = 0;
        msgAck.ack.code = (ub2_)ETransactionExitReason::NO_THE_SESSION_FOUND;
        __send((Message::TMsg *)&msgAck, false_v);

        return over(ETransactionExitReason::NO_THE_SESSION_FOUND);
    }

    if (false_v == CTransactionManager::instance()->registerTransaction(this)) {
        log_notice(
            "[%p]CTransaction::onStart: there is an transaction with "
            "the same sessionId-%s and id-%llu",
            this, _sessionId, _id);
        memset(_sessionId, 0, Size::SESSION_ID);
        _id = 0;
        msgAck.ack.code = (ub2_)ETransactionExitReason::SAME_SESSION_ID;
        __send((Message::TMsg *)&msgAck, false_v);

        return over(ETransactionExitReason::SAME_SESSION_ID);
    }

    _status = ETransactionStatus::READY;
    msgAck.ack.code = 0;
    _lastUpdate = msg->lastUpdate;

    if (true_v == __send((Message::TMsg *)&msgAck, false_v)) {
        _heartbeat = true_v;
        _keepLiveTimerId = CTransactionManager::instance()->setTimer(
            Config::App::HEARTBEAT_INTERVAL, this, (obj_)_status, 0);

        if (0 == _keepLiveTimerId) {
            return over(ETransactionExitReason::NO_MORE_TIMER);
        }
    }

    return true_v;
}

bool_ CTransaction::__onHeartBeat(const Message::TPDUHeartBeatAck *msg) {
    log_debug("[%p]CTransaction::onHeartBeat: current status-%d", this,
              (int)_status);

    if (ETransactionStatus::READY != _status) {
        return over(ETransactionExitReason::WRONG_STATUS);
    }

    _heartbeat = true_v;

    return true_v;
}

// Called by CNodeGroup thread
bool_ CTransaction::__onSendMsg(const Message::TPDUSendMsg *msg) {
    log_debug("[%p]CTransaction::onSendMsg: current status-%d", this,
              (int)_status);

    if (ETransactionStatus::READY != _status) {
        return over(ETransactionExitReason::WRONG_STATUS);
    }

    Message::TPDUSendMsgAck msgAck;

    memcpy(&msgAck.header, &msg->header, sizeof(Message::THeader));
    msgAck.header.size = sizeof(Message::TPDUSendMsgAck);
    msgAck.header.type |= Message::MT_SIGN_ACK;
    msgAck.header.stmp = CBase::now();
    msgAck.ack.code = 0;

    if (false_v ==
        _node->getGroup()->ro().sendMessage(this, msg, msgAck.messageId)) {
        msgAck.ack.code = (ub2_)ETransactionExitReason::NO_DESTINATION_FOUND;
    }

    __send((Message::TMsg *)&msgAck, false_v);

    return true_v;
}

bool_ CTransaction::handlePushMessage(ub1_ ornType, ub8_ ornId, ub8_ ornExtId,
                                      ub8_ messageId, const c1_ *json,
                                      ub2_ size, ub8_ timestamp) {
    Message::TPDUPushMsg msg;

    msg.header.size = sizeof(Message::TPDUPushMsg);
    msg.header.type = Message::MT_SERVICE;
    msg.header.cmd = Message::MC_PUSH_MSG;
    msg.header.ver = Config::App::PROTOCOL_VERSION;
    msg.header.lang = Message::ML_CN;
    msg.header.seq = 0;
    msg.header.stmp = CBase::now();
    msg.header.ext = 0;
    msg.ornType = ornType;
    msg.ornId = ornId;
    msg.ornExtId = ornExtId;
    msg.messageId = messageId;
    memset(msg.json, 0, Size::JSON);
    assert(Length::JSON >= size);
    strncpy(msg.json, json, Length::JSON);

    bool_ ret = __send((Message::TMsg *)&msg, true_v);

    if (true_v == ret) {
        _lastUpdate = timestamp;
    }

    return ret;
}

// Called by CNodeGroup thread
bool_ CTransaction::__onPushMsg(const Message::TPDUPushMsgAck *msg) {
    log_debug("[%p]CTransaction::onPushMsg: current status-%d", this,
              (int)_status);

    if (ETransactionStatus::READY != _status) {
        return over(ETransactionExitReason::WRONG_STATUS);
    }

    assert(msg->header.seq);
    if (0 != msg->header.seq) {
        MapSeq2Timer::iterator pos = _mapSeq2Timer.find(msg->header.seq);

        if (_mapSeq2Timer.end() != pos) {
            CTransactionManager::instance()->killTimer(pos->second);
            _mapSeq2Timer.erase(pos);
        }

        return true_v;
    }

    log_error("[%p]CTransaction::onPushMsg: no sequence found in push msg ack",
              this);

    return false_v;
}

bool_ CTransaction::__onTimer(const Message::TPDUOnTimer *msg) {
    if (_keepLiveTimerId == msg->timerId) {
        if (_status != (ETransactionStatus)msg->parameter) {
            log_debug(
                "[%p]CTransaction::onTimer: timer status(%d) != "
                "current(%d) status",
                this, (int)msg->parameter, (int)_status);

            return true_v;
        }

        if (ETransactionStatus::CONNECTED == _status) {
            log_debug(
                "[%p]CTransaction::onTimer: waiting handshake timeout, "
                "current status-%d",
                this, (int)_status);

            return over(ETransactionExitReason::TIME_OUT);
        } else if (ETransactionStatus::READY == _status) {
            if (false_v == _heartbeat) {
                log_debug(
                    "[%p]CTransaction::onTimer: heartbeat timeout, "
                    "current status-%d",
                    this, (int)_status);

                return over(ETransactionExitReason::TIME_OUT);
            } else {
                Message::TPDUHeartBeat message;

                message.header.size = sizeof(Message::TPDUHeartBeat);
                message.header.type = Message::MT_CONTROL;
                message.header.cmd = Message::MC_HEART_BEAT;
                message.header.ver = Config::App::PROTOCOL_VERSION;
                message.header.lang = 1;
                message.header.seq = 0;

                __send((Message::TMsg *)&message, false_v);
                _heartbeat = false_v;
            }
        } else {
            assert(false_v);
        }
    } else {
        if (ETransactionStatus::READY != _status) {
            log_debug(
                "[%p]CTransaction::onTimer: seq(%u)'s status(%d) is "
                "expected, but now it's status(%d)",
                this, (unsigned int)msg->parameter,
                (int)ETransactionStatus::READY, (int)_status);

            return true_v;
        }

        ub4_ seq = (ub4_)msg->parameter;
        MapSeq2Timer::iterator pos = _mapSeq2Timer.find(seq);

        if (_mapSeq2Timer.end() != pos) {
            log_debug(
                "[%p]CTransaction::onTimer: waitting ack timeout, "
                "current status-%d, seq-%u",
                this, (int)_status, seq);

            return over(ETransactionExitReason::TIME_OUT);
        } else {
            assert(false_v);
        }
    }

    return true_v;
}

bool_ CTransaction::__onStop(const Message::TPDUOnOver *msg) {
    log_debug("[%p]CTransaction::onStop: current status-%d", this,
              (int)_status);
    _status = ETransactionStatus::OVER;

    switch ((ETransactionExitReason)msg->reason) {
        case ETransactionExitReason::WRONG_STATUS:
            log_debug("[%p]CTransaction::onStop: reason-%s", this,
                      "wrong status");
            break;
        case ETransactionExitReason::CLIENT_TOO_OLD:
            log_debug("[%p]CTransaction::onStop: reason-%s", this,
                      "client too old");
            break;
        case ETransactionExitReason::NO_THE_SESSION_FOUND:
            log_debug("[%p]CTransaction::onStop: reason-%s", this,
                      "no the session found");
            break;
        case ETransactionExitReason::SAME_SESSION_ID:
            log_debug("[%p]CTransaction::onStop: reason-%s", this,
                      "same session id");
            break;
        case ETransactionExitReason::TIME_OUT:
            log_debug("[%p]CTransaction::onStop: reason-%s", this, "time out");
            break;
        case ETransactionExitReason::UNKNOWN_MESSAGE:
            log_debug("[%p]CTransaction::onStop: reason-%s", this,
                      "unknown message");
            break;
        case ETransactionExitReason::CONNECTION_BROKEN:
            log_debug("[%p]CTransaction::onStop: reason-%s", this,
                      "connection broken");
            break;
        case ETransactionExitReason::CONNECTION_ERROR:
            log_debug("[%p]CTransaction::onStop: reason-%s", this,
                      "connection error");
            break;
        case ETransactionExitReason::CANNOT_RECV_DATA:
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
    }

    _build = 0;
    _lastUpdate = 0;
    _id = 0;
    _heartbeat = false_v;
    _seqCounter = 0;

    return false_v;
}

bool_ CTransaction::__send(Message::TMsg *msg, bool_ waitAck) {
    assert(null_v != msg);
    msg->stmp = CBase::now();
    msg->ext = 0;

    if (true_v == waitAck) {
        ub4_ seq = ++_seqCounter;

        if (0 == seq) {
            seq = ++_seqCounter;
        }

        msg->seq = seq;
    }

    bool_ ret = _node->send(msg);

    if (true_v == ret && true_v == waitAck) {
        ub8_ timerId = CTransactionManager::instance()->setTimer(
            Config::App::HEARTBEAT_INTERVAL, this, (obj_)(ub8_)msg->seq, 1);

        if (timerId) {
            _mapSeq2Timer[msg->seq] = timerId;
        } else {
            return over(ETransactionExitReason::NO_MORE_TIMER);
        }
    }

    return ret;
}
