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
#include "../common/CLoopBuffer.h"
#include "../database/CRedisOperator.h"
#include "Message.h"

class CNode;

#include <list>

typedef std::list<CNode *> Container;

class CNodeGroup: public CBase, public IWorkable {
public:
	CNodeGroup();
	virtual ~CNodeGroup();

	// called by CTrafficManager thread
	bool_ attach(CNode *node, const c1_ *ip, ub2_ port, b4_ fd);

	// called by CTrafficManager thread
	none_ detach(CNode *node);

	// called by CTrafficManager thread or CTransactionManager thread
	bool_ putMessage(const Message::TMsg *msg);

	CRedisOperator &ro() {
		return _ro;
	}

	virtual bool_ working();

private:
	CWorker _worker;
	CMutex _mutex;

	CLoopBuffer _queue;
	ub1_ _buffer[Message::MSG_MAX_LENGTH];

	CRedisOperator _ro;

	Container _container;
	Container::iterator _pos;

	bool_ _rollingQueue();
	bool_ _rollingNode();
};

#endif // _C_NODE_GROUP_H_
