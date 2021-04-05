/*
 ============================================================================
 Name        : CRedisOperator.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _C_REDIS_OPERATOR_H_
#define _C_REDIS_OPERATOR_H_

#include <hiredis/hiredis.h>

#include "../common/CBase.h"
#include "../traffic/Message.h"

class CTransaction;

class CRedisOperator : public CBase {
public:
    CRedisOperator();
    virtual ~CRedisOperator();

    ub8_ verifyHandshake(const c1_ *sessionId);
    bool_ sendMessage(const CTransaction *transaction,
                      const Message::TPDUSendMsg *msg, ub8_ &messageId);
    bool_ checkMessages(CTransaction *transaction);

    bool_ connect();
    none_ disconnect();

private:
    redisContext *_context;

    bool_ _errorHandler(obj_ reply, bool_ freeReply = true_v);
};

#endif  // _C_REDIS_OPERATOR_H_
