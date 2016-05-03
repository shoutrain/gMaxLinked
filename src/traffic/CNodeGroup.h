/*
 ============================================================================
 Name        : CNodeGroup.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _C_NODE_GROUP_H_
#define _C_NODE_GROUP_H_

#include "../common/CBase.h"
#include "../common/IWorkable.h"
#include "../common/CWorker.h"
#include "../common/CMutex.h"
#include "../common/CCond.h"
#include "../common/CLoopBuffer.h"
#include "../database/CRedisOperator.h"
#include "Message.h"

class CNode;

class CNodeGroup: public CBase, public IWorkable {
public:
	CNodeGroup();
	virtual ~CNodeGroup();

	// called by CTrafficManager thread
	void attach(CNode *node, const char *ip, unsigned short port, int fd);

	// called by CTrafficManager thread
	void detach(CNode *node);

	// called by CTrafficManager and CTransactionManager threads
	bool putMessage(const Message::TMsg *msg);

	CRedisOperator &ro() {
		return _ro;
	}

protected:
	virtual bool working();

private:
	CWorker _worker;

	CMutex _mutex;
	CCond _cond;

	CLoopBuffer _queue;
	Message::TMsg _curMsg;

	CRedisOperator _ro;

	unsigned int _nodeNum;
};

#endif // _C_NODE_GROUP_H_
