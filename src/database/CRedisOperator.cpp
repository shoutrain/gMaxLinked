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
#include "../transaction/CTransaction.h"

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

	redisReply *reply = (redisReply *) redisCommand(_context, "AUTH %s",
			Config::Redis::AUTH);

	if (!_errorHandler(reply, true_v)) {
		return false_v;
	}

	reply = (redisReply *) redisCommand(_context, "SELECT %u",
			Config::Redis::DB);

	if (!_errorHandler(reply, true_v)) {
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

bool_ CRedisOperator::_errorHandler(obj_ reply, bool_ freeReply) {
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

ub8_ CRedisOperator::verifyHandshake(const c1_ *sessionId) {
	assert(sessionId && 0 != sessionId[0]);
	if (!sessionId || 0 == sessionId[0]) {
		return 0;
	}

	assert(_context);
	if (null_v == _context) {
		log_fatal("CRedisOperator::verifyHandshake: no context");

		return 0;
	}

	redisReply *reply = (redisReply *) redisCommand(_context,
			"zscore gml:account:session %s", sessionId);

	if (!_errorHandler(reply, false_v)) {
		return 0;
	}

	if (REDIS_REPLY_NIL == reply->type) {
		freeReplyObject(reply);

		return 0;
	}

	assert(REDIS_REPLY_STRING == reply->type);
	ub8_ id = (ub8_) atoll(reply->str);

	freeReplyObject(reply);

	return id;
}

bool_ CRedisOperator::sendMessage(const CTransaction *transaction,
		const Message::TPDUSendMsg *msg, ub8_ &messageId) {
	assert(transaction);
	assert(msg);
	assert(_context);
	if (null_v == _context) {
		log_fatal("CRedisOperator::sendMessage: no context");

		return false_v;
	}

	redisReply *reply = (redisReply *) redisCommand(_context,
			"hincrby gml:message:key:generator messageId 1");

	if (!_errorHandler(reply, false_v)) {
		return false_v;
	}

	assert(REDIS_REPLY_INTEGER == reply->type);
	messageId = (ub8_) reply->integer;

	freeReplyObject(reply);

	ub8_ ornExtId = 0;

	reply = (redisReply *) redisCommand(_context,
			"hmset gml:message:%lu ornType %u ornId %lu ornExtId %lu json %s",
			messageId, msg->dstType, transaction->getId(), msg->dstId,
			msg->json);

	if (!_errorHandler(reply, true_v)) {
		return false_v;
	}

	if (1 == msg->dstType) {
		reply = (redisReply *) redisCommand(_context,
				"zadd gml:message:queue:user:%lu %lu %lu", msg->dstId,
				CBase::now(), messageId);

		if (!_errorHandler(reply, true_v)) {
			return false_v;
		}
	} else if (2 == msg->dstType) {
		ub8_ now = CBase::now();

		reply = (redisReply *) redisCommand(_context,
				"zadd gml:message:queue:group:%lu %lu %lu", msg->dstId, now,
				messageId);

		if (!_errorHandler(reply, true_v)) {
			return false_v;
		}

		reply = (redisReply *) redisCommand(_context,
				"zrangebyscore gml:group:members:%lu 1 1", msg->dstId);

		if (!_errorHandler(reply, false_v)) {
			return false_v;
		}

		// there is no members in the group, just ignore
		if (REDIS_REPLY_NIL == reply->type) {
			freeReplyObject(reply);

			return true_v;
		}

		assert(REDIS_REPLY_ARRAY == reply->type);
		b4_ i = 0;

		for (; i < reply->elements; i++) {
			assert(REDIS_REPLY_STRING == reply->element[i]->type);
			ub8_ userId = (ub8_) atoll(reply->element[i]->str);
			redisReply *subReply = (redisReply *) redisCommand(_context,
					"zadd gml:message:queue:user:%lu %lu %lu", userId, now,
					messageId);
			// do not check if it's successful

			freeReplyObject(subReply);
		}

		freeReplyObject(reply);
	}

	return true_v;
}

bool_ CRedisOperator::checkMessages(CTransaction *transaction) {
	assert(transaction);
	assert(_context);
	if (null_v == _context) {
		log_fatal("CRedisOperator::checkMessages: no context");

		return false_v;
	}

	ub8_ now = CBase::now();
	assert(now > transaction->getLastUpdate());
	redisReply *reply;

	if (0 == transaction->getLastUpdate()
			|| Config::Msg::VALID_DURATION
					< now - transaction->getLastUpdate()) {
		redisReply *subReply = (redisReply *) redisCommand(_context,
				"zcount gml:message:queue:user:%lu -inf +inf",
				transaction->getId());

		if (!_errorHandler(subReply, false_v)) {
			return false_v;
		}

		assert(REDIS_REPLY_INTEGER == subReply->type);
		int num = subReply->integer;
		freeReplyObject(subReply);

		if (0 >= num) {
			return false_v;
		} else if (Config::Msg::VALID_NUMBER <= num) {
			num -= Config::Msg::VALID_NUMBER;
		} else {
			num = 0;
		}

		reply = (redisReply *) redisCommand(_context,
				"zrangebyscore gml:message:queue:user:%lu -inf +inf withscores "
						"limit %d %u", transaction->getId(), num,
				Config::Msg::VALID_NUMBER);
	} else {
		reply = (redisReply *) redisCommand(_context,
				"zrangebyscore gml:message:queue:user:%lu (%lu +inf withscores "
						"limit 0 %u", transaction->getId(),
				transaction->getLastUpdate(), Config::Msg::VALID_DURATION);
	}

	if (!_errorHandler(reply, false_v)) {
		return false_v;
	}

	if (REDIS_REPLY_NIL == reply->type) {
		freeReplyObject(reply);

		return false_v;
	}

	assert(REDIS_REPLY_ARRAY == reply->type);
	b4_ n = reply->elements / 2;

	for (b4_ i = 0; i < n; i++) {
		assert(REDIS_REPLY_STRING == reply->element[i * 2]->type);
		assert(REDIS_REPLY_STRING == reply->element[i * 2 + 1]->type);
		redisReply * subReply = (redisReply *) redisCommand(_context,
				"hmget gml:message:%s ornType ornId ornExtId json",
				reply->element[i * 2]->str);

		if (!_errorHandler(subReply, false_v)) {
			freeReplyObject(reply);

			return false_v;
		}

		assert(REDIS_REPLY_ARRAY == subReply->type);
		assert(4 == subReply->elements);
		if (REDIS_REPLY_NIL == subReply->element[0]->type
				|| REDIS_REPLY_NIL == subReply->element[1]->type
				|| REDIS_REPLY_NIL == subReply->element[2]->type
				|| REDIS_REPLY_NIL == subReply->element[3]->type) {
			freeReplyObject(subReply);

			continue;
		}

		ub1_ ornType = (ub1_) atoi(subReply->element[0]->str);
		ub8_ ornId = (ub8_) atoll(subReply->element[1]->str);
		ub8_ ornExtId = (ub8_) atoll(subReply->element[2]->str);
		bool_ result = transaction->handlePushMessage(ornType, ornId, ornExtId,
				(ub8_) atoll(reply->element[i * 2]->str),
				subReply->element[3]->str, subReply->element[3]->len,
				(ub8_) atoll(reply->element[i * 2 + 1]->str));

		if (false_v == result) {
			freeReplyObject(subReply);
			freeReplyObject(reply);

			return false_v;
		}

		freeReplyObject(subReply);
	}

	freeReplyObject(reply);

	return true_v;
}
