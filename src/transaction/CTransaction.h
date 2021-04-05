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

#include <map>

#include "../common/CBase.h"
#include "../traffic/Message.h"

class CNode;
class TTimer;

// FREE->CONNECTED->READY->OVER->FREE
enum class ETransactionStatus { FREE, CONNECTED, READY, OVER };

enum class ETransactionExitReason {
    WRONG_STATUS = 1,
    CLIENT_TOO_OLD,
    TIME_OUT,
    UNKNOWN_MESSAGE,
    NO_THE_SESSION_FOUND,
    SAME_SESSION_ID,
    CONNECTION_BROKEN,
    CONNECTION_ERROR,
    CANNOT_RECV_DATA,
    NO_DESTINATION_FOUND,
    NO_MORE_TIMER,
    NO_MORE_QUEUE_SPACE
};

typedef std::map<ub4_, ub8_> MapSeq2Timer;

class CTransaction : public CBase {
public:
    CTransaction(CNode *node);
    virtual ~CTransaction();

    // Called by CTrafficManager thread
    bool_ onAttach();

    // Called by CTrafficManager thread
    none_ onDetach();

    // Called by CNodeGroup thread
    bool_ onMessage(const Message::TMsg *msg);

    // Called by CNodeGroup thread
    void onCheck();

    CNode *getNode() { return _node; }

    ETransactionStatus getStatus() const { return _status; }

    const c1_ *getSessionId() const { return _sessionId; }

    ub8_ getId() const { return _id; }

    ub8_ getLastUpdate() const { return _lastUpdate; }

    bool_ handlePushMessage(ub1_ ornType, ub8_ ornId, ub8_ ornExtId,
                            ub8_ messageId, const c1_ *json, ub2_ size,
                            ub8_ timestamp);

    // Called by other threads except CNodeGroup thread when useQueue == true_v
    // Called by CNodeGroup thread when useQueue == false_v
    bool_ over(ETransactionExitReason reason, bool_ useQueue = false_v);

protected:
    // Called by CNodeGroup thread
    bool_ __onStart(const Message::TPDUHandShake *msg);

    // Called by CNodeGroup thread
    bool_ __onHeartBeat(const Message::TPDUHeartBeatAck *msg);

    // Called by CNodeGroup thread
    bool_ __onSendMsg(const Message::TPDUSendMsg *msg);

    // Called by CNodeGroup thread
    bool_ __onPushMsg(const Message::TPDUPushMsgAck *msg);

    // Called by CNodeGroup thread
    bool_ __onTimer(const Message::TPDUOnTimer *data);

    // Called by CNodeGroup thread
    bool_ __onStop(const Message::TPDUOnOver *data);

    // Cassed by CNodeGroup thread
    bool_ __send(Message::TMsg *msg, bool_ waitAck);

private:
    CNode *_node;

    ETransactionStatus _status;

    ub4_ _build;
    ub8_ _lastUpdate;
    c1_ _sessionId[Size::SESSION_ID];
    ub8_ _id;

    // for heartbeat checking
    ub8_ _keepLiveTimerId;
    bool_ _heartbeat;

    // for message ack
    ub4_ _seqCounter;
    MapSeq2Timer _mapSeq2Timer;
};

#endif  // _C_TRANSACTION_H_
