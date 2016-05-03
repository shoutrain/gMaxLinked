/*
 ============================================================================
 Name        : CTrafficManager.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _C_TRAFFIC_MANAGER_H_
#define _C_TRAFFIC_MANAGER_H_

#include "../common/IWorkable.h"
#include "../common/CWorker.h"
#include "../common/CResource.h"
#include "../common/CMutex.h"
#include <queue>

class CTransactionMangaer;
class CNode;
class CNodeGroup;

typedef std::queue<CNode *> NodeQueue;

class CTrafficManager: public CBase, public IWorkable {
public:
	static CTrafficManager *instance();
	static void destory();

	void work();

	// called by CNodeGroup thread
	void recycleNode(CNode *node);

protected:
	virtual bool working();

private:
	static CTrafficManager *_tm;

	CTrafficManager();
	virtual ~CTrafficManager();

	void addNodes();
	void delNode(CNode *node);

	static void setNonBlocking(int iSocket);

	CNodeGroup *allocateGroup();

	bool _running;

	int _listenFd;
	int _epollFd;

	// store all nodes
	CResource<CNode, CTrafficManager> _res;

	// store all groups
	CNodeGroup *_groups;
	unsigned int _curGroupIndex;

	CWorker _worker;

	// store all nodes which will be recycled
	NodeQueue _nodeQueue;
	CMutex _mutex;
};

#endif // _C_TRAFFIC_MANAGER_H_
