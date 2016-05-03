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
	void onAttach(CNodeGroup *node, const char *ip, unsigned short port,
			int fd);

	// called when CNodeGroup detach the node
	// called by CTrafficManager thread
	void onDetach();

	// receive data from client and put them into the input queue
	// called by CTrafficManager thread
	bool recv();

	// send data to client
	// called by CNodeGroup thread
	bool send(const Message::TMsg *msg);

	const char *getIp() const {
		return _ip;
	}

	unsigned short getPort() const {
		return _port;
	}

	int getFd() const {
		return _fd;
	}

	CTransaction *getTransaction() {
		return &_transaction;
	}

	CNodeGroup *getGroup() {
		return _group;
	}

private:
	char _ip[Size::IP_V4];
	unsigned short _port;

	// socket file descriptor
	int _fd;

	// corresponding unite on upper layer
	CTransaction _transaction;

	// thread to run the node and corresponding transaction
	CNodeGroup *_group;

	// message buffer
	unsigned int _recvOffset;
	unsigned char _recvBuffer[Message::MSG_FIXED_LENGTH];
};

#endif // _C_NODE_H_
