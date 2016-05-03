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

#include "../common/CBase.h"
#include "../traffic/Message.h"

#include <hiredis/hiredis.h>

class CRedisOperator: public CBase {
public:
	CRedisOperator();
	virtual ~CRedisOperator();

	bool connect();
	void disconnect();



private:
	redisContext *_context;

	bool errorHandler(void *reply, bool freeReply = true);
};

#endif // _C_REDIS_OPERATOR_H_
