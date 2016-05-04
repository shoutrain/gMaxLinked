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
	_context = null_v;
}

CRedisOperator::~CRedisOperator() {
	if (_context) {
		redisFree(_context);
	}
}

bool_ CRedisOperator::connect() {
	if (_context) {
		redisFree(_context);
	}

	timeval timeout = { Config::Redis::TIMEOUT, 0 };

	_context = redisConnectWithTimeout(Config::Redis::HOST, Config::Redis::PORT,
			timeout);

	if (null_v == _context) {
		log_fatal("CRedisOperator:connect: cannot allocate redis context");

		return false_v;
	}

	if (_context->err) {
		log_fatal("CRedisOperator::connect: failed to call "
				"redisConnectWithTimeout-%s", _context->errstr);
		redisFree(_context);
		_context = null_v;

		return false_v;
	}

	return true_v;
}

none_ CRedisOperator::disconnect() {
	if (_context) {
		redisFree(_context);
		_context = null_v;
	}
}

bool_ CRedisOperator::errorHandler(obj_ reply, bool_ freeReply) {
	if (null_v == _context) {
		return false_v;
	}

	bool_ ret = true_v;
	redisReply *rr = (redisReply *) reply;

	if (!rr) {
		log_error("CRedisOperator::errorHandler: %s", _context->errstr);
		connect(); // reconnect
		ret = false_v;
	} else {
		if (REDIS_REPLY_ERROR == rr->type) {
			log_error("CRedisOperator::errorHandler: %s", rr->str);
			ret = false_v;
			freeReplyObject(rr);
		} else if (freeReply) {
			freeReplyObject(rr);
		}
	}

	return ret;
}
