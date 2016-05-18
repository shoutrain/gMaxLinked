/*
 ============================================================================
 Name        : CTransaction.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _C_TRANSACTION_H_
#define _C_TRANSACTION_H_

#include "../common/CBase.h"
#include "../traffic/Message.h"

#include <map>

class CNode;
class TTimer;

// FREE->CONNECTED->READY->OVER->FREE
enum ETransactionStatus {
	FREE, CONNECTED, READY, OVER
};

enum ETransactionExitReason {
	WRONG_STATUS = 1,
	CLIENT_TOO_OLD,
	TIME_OUT,
	UNKNOWN_MESSAGE,
	NO_THE_SESSION_FOUND,
	SAME_SESSION_ID,
	CONNECTION_BROKEN,
	CONNECTION_ERROR,
	CANNOT_RECV_DATA
};

typedef std::map<ub4_, ub8_> MapSeq2Timer;

class CTransaction: public CBase {
public:
	CTransaction(CNode *node);
	virtual ~CTransaction();

	// Called by CTrafficManager thread
	none_ onAttach();

	// Called by CTrafficManager thread
	none_ onDetach();

	// Called by CNodeGroup thread
	bool_ onMessage(const Message::TMsg *msg);

	CNode *getNode() {
		return _node;
	}

	ETransactionStatus getStatus() const {
		return _status;
	}

	const c1_ *getSessionId() const {
		return _sessionId;
	}

	ub8_ getId() const {
		return _id;
	}

	none_ over(ETransactionExitReason reason);

protected:
	// Called by CNodeGroup thread
	bool_ __onStart(const Message::TPDUHandShake *msg);

	// Called by CNodeGroup thread
	bool_ __onHeartBeat(const Message::TPDUHeartBeat *msg);

	// Called by CNodeGroup thread
	bool_ __onTimer(const Message::TPDUOnTimer *data);

	// Called by CNodeGroup thread
	bool_ __onStop(const Message::TPUDOnOver *data);

	bool_ __send(Message::TMsg *msg, bool_ waitAck, bool_ interval = false_v);

private:
	CNode *_node;

	ETransactionStatus _status;

	c1_ _sessionId[Size::SESSION_ID];
	ub8_ _id;

	// for heartbeat checking
	ub8_ _keepLiveTimerId;
	bool_ _heartbeat;

	// for message ack
	ub4_ _seqCounter;
	MapSeq2Timer _mapSeq2Timer;

};

#endif // _C_TRANSACTION_H_
