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
	none_ attach(CNode *node, const s1_ ip, ub2_ port, b4_ fd);

	// called by CTrafficManager thread
	none_ detach(CNode *node);

	// called by CTrafficManager and CTransactionManager threads
	bool_ putMessage(const Message::TMsg *msg);

	CRedisOperator &ro() {
		return _ro;
	}

protected:
	virtual bool_ working();

private:
	CWorker _worker;

	CMutex _mutex;
	CCond _cond;

	CLoopBuffer _queue;
	Message::TMsg _curMsg;

	CRedisOperator _ro;

	ub4_ _nodeNum;
};

#endif // _C_NODE_GROUP_H_