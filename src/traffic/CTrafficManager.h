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

#include <queue>

#include "../common/CMutex.h"
#include "../common/CResource.h"
#include "../common/CWorker.h"
#include "../common/IWorkable.h"

class CTransactionMangaer;
class CNode;
class CNodeGroup;

typedef std::queue<CNode *> NodeQueue;

class CTrafficManager : public CBase, public IWorkable {
public:
    static CTrafficManager *instance();
    static none_ destory();

    none_ work();

    // called by CNodeGroup thread
    none_ recycleNode(CNode *node);

    virtual bool_ working();

private:
    static CTrafficManager *_tm;

    CTrafficManager();
    virtual ~CTrafficManager();

    none_ _addNodes();
    none_ _delNode(CNode *node);

    static none_ _setNonBlocking(b4_ socket);

    CNodeGroup *_allocateGroup();

    bool_ _running;

    b4_ _listenFd;
    b4_ _epollFd;

    // store all nodes
    CResource<CNode, CTrafficManager> _res;

    // store all groups
    CNodeGroup *_groups;
    ub4_ _curGroupIndex;

    CWorker _worker;

    // store all nodes which will be recycled
    NodeQueue _nodeQueue;
    CMutex _mutex;
};

#endif  // _C_TRAFFIC_MANAGER_H_
