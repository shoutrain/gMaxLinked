/*
 ============================================================================
 Name        : CNode.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _C_NODE_H_
#define _C_NODE_H_

#include "../transaction/CTransaction.h"
#include "Message.h"
#include "CNodeGroup.h"

class CNode: public CBase {
public:
	CNode();
	virtual ~CNode();

	// called when CNodeGroup attach the node
	// called by CTrafficManager thread
	none_ onAttach(CNodeGroup *node, const s1_ ip, ub2_ port, b4_ fd);

	// called when CNodeGroup detach the node
	// called by CTrafficManager thread
	none_ onDetach();

	// receive data from client and put them b4_o the input queue
	// called by CTrafficManager thread
	bool_ recv();

	// send data to client
	// called by CNodeGroup thread
	bool_ send(const Message::TMsg *msg);

	const s1_ getIp() const {
		return _ip;
	}

	ub2_ getPort() const {
		return _port;
	}

	b4_ getFd() const {
		return _fd;
	}

	CTransaction *getTransaction() {
		return &_transaction;
	}

	CNodeGroup *getGroup() {
		return _group;
	}

private:
	c1_ _ip[Size::IP_V4];
	ub2_ _port;

	// socket file descriptor
	b4_ _fd;

	// corresponding unite on upper layer
	CTransaction _transaction;

	// thread to run the node and corresponding transaction
	CNodeGroup *_group;

	// message buffer
	ub4_ _recvOffset;
	ub1_ _recvBuffer[Message::MSG_FIXED_LENGTH];
};

#endif // _C_NODE_H_