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
class CRedisOperator;

// FREE->CONNECTED->READY->OVER->FREE
enum ETransactionStatus {
	FREE, CONNECTED, READY, OVER
};

enum ETransactionExitReason {
	WRONG_STATUS,
	CLIENT_TOO_OLD,
	TIME_OUT,
	UNKNOWN_MESSAGE,
	SAME_TRANSACTION_ID,
	CONNECTION_BROKEN,
	CONNECTION_ERROR,
	CANNOT_RECV_DATA
};

class CTransaction: public CBase {
public:
	CTransaction(CNode *node);
	virtual ~CTransaction();

	// Called by CTrafficManager thread
	void onAttach();

	// Called by CTrafficManager thread
	void onDetach();

	// Called by CNodeGroup thread
	bool onMessage(const Message::TMsg *msg);

	CNode *getNode() {
		return _node;
	}

	ETransactionStatus getStatus() const {
		return _status;
	}

	unsigned int getId() const {
		return _id;
	}

	void over(ETransactionExitReason reason);

protected:
	bool onStart(unsigned short length, unsigned int sequence,
			const Message::TMBHandshake *data);
	bool onRealtime(unsigned int sequence, const Message::TMBRealtime *data);
	bool onTimer(const Message::TMBTimer *data);
	bool onStop(const Message::TMBOver *data);

private:
	CNode *_node;

	ETransactionStatus _status;

	unsigned int _id;

	// store timer for heartbeat checking
	long unsigned int _keepLiveTimerId;
};

#endif // _C_TRANSACTION_H_
