/*
 ============================================================================
 Name        : CRedisOperator.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#include "CRedisOperator.h"
#include "../config/Config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

CRedisOperator::CRedisOperator() {
	_context = NULL;
}

CRedisOperator::~CRedisOperator() {
	if (_context) {
		redisFree(_context);
	}
}

bool CRedisOperator::connect() {
	if (_context) {
		redisFree(_context);
	}

	timeval timeout = { Config::Redis::TIMEOUT, 0 };

	_context = redisConnectWithTimeout(Config::Redis::HOST, Config::Redis::PORT,
			timeout);

	if (NULL == _context) {
		log_fatal("CRedisOperator:connect: cannot allocate redis context");

		return false;
	}

	if (_context->err) {
		log_fatal("CRedisOperator::connect: failed to call "
				"redisConnectWithTimeout-%s", _context->errstr);
		redisFree(_context);
		_context = NULL;

		return false;
	}

	return true;
}

void CRedisOperator::disconnect() {
	if (_context) {
		redisFree(_context);
		_context = NULL;
	}
}

bool CRedisOperator::errorHandler(void *reply, bool freeReply) {
	if (NULL == _context) {
		return false;
	}

	bool ret = true;
	redisReply *rr = (redisReply *) reply;

	if (!rr) {
		log_error("CRedisOperator::errorHandler: %s", _context->errstr);
		connect(); // reconnect
		ret = false;
	} else {
		if (REDIS_REPLY_ERROR == rr->type) {
			log_error("CRedisOperator::errorHandler: %s", rr->str);
			ret = false;
			freeReplyObject(rr);
		} else if (freeReply) {
			freeReplyObject(rr);
		}
	}

	return ret;
}
