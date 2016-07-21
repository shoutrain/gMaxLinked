/*
 ============================================================================
 Name        : Config.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#define GLOBAL_CONFIG
#include "Config.h"

#include <src/common/CConfReader.h>
#include "DefaultConfig.h"

#include <string.h>

namespace Config {

namespace App {

c1_ CONF_FILE[Size::PATH];
ub4_ RUN_AS_DAEMON;
c1_ NAME[Size::NAME];
ub4_ BASE_BUILD;
ub2_ PROTOCOL_VERSION;
c1_ LISTEN_IP[Size::IP_V4];
ub2_ LISTEN_PORT;
ub4_ EPOLL_WAIT_EVENT_NUM;
ub4_ NODE_GROUP_NUM;
ub4_ NODE_GROUP_SIZE;
ub4_ TOTAL_SUPPORT_USER_NUM;
ub4_ TOTAL_THREAD_NUM;
ub4_ MESSAGE_MAX_NUM_IN_QUEUE;
ub4_ THREAD_STACK_SIZE;
ub4_ HEARTBEAT_INTERVAL;

}

namespace Msg {

ub8_ VALID_DURATION;
ub1_ VALID_NUMBER;

}

namespace Redis {

c1_ HOST[Size::URL];
ub2_ PORT;
ub4_ TIMEOUT;
c1_ AUTH[Size::PASSWORD];
ub1_ DB;

}

none_ initializeApp(CConfReader *conf);
none_ initializeMsg(CConfReader *conf);
none_ initializeRedis(CConfReader *conf);

none_ initialize(const c1_ *confFileName) {
	if (null_v != confFileName && 0 < strlen(confFileName)
			&& Size::PATH > strlen(confFileName)) {
		strncpy(App::CONF_FILE, confFileName, Size::PATH);
	} else {
		strncpy(App::CONF_FILE, DefaultConfig::App::CONF_FILE, Size::PATH);
	}

	CConfReader *conf = new CConfReader(App::CONF_FILE);

	initializeApp(conf);
	initializeMsg(conf);
	initializeRedis(conf);
}

none_ initializeApp(CConfReader *conf) {
	assert(null_v != conf);
	const c1_ *section = "app";

	if (conf->readInt(section, "run_as_daemon", &App::RUN_AS_DAEMON)
			|| 0 == App::RUN_AS_DAEMON) {
		App::RUN_AS_DAEMON = DefaultConfig::App::RUN_AS_DAEMON;
	}

	if (conf->readString(section, "name", App::NAME, Size::NAME)
			|| 0 == App::NAME[0]) {
		strncpy(App::NAME, DefaultConfig::App::NAME, Size::NAME);
	}

	if (conf->readInt(section, "base_build", &App::BASE_BUILD)
			|| 0 == App::BASE_BUILD) {
		App::BASE_BUILD = DefaultConfig::App::BASE_BUILD;
	}

	if (conf->readShort(section, "protocol_version", &App::PROTOCOL_VERSION)
			|| 0 == App::PROTOCOL_VERSION) {
		App::PROTOCOL_VERSION = DefaultConfig::App::PROTOCOL_VERSION;
	}

	if (conf->readString(section, "listen_ip", App::LISTEN_IP, Size::IP_V4)
			|| 0 == App::LISTEN_IP[0]) {
		strncpy(App::LISTEN_IP, DefaultConfig::App::LISTEN_IP, Size::IP_V4);
	}

	if (conf->readShort(section, "listen_port", &App::LISTEN_PORT)
			|| 0 == App::LISTEN_PORT) {
		App::LISTEN_PORT = DefaultConfig::App::LISTEN_PORT;
	}

	if (conf->readInt(section, "epoll_wait_event_number",
			&App::EPOLL_WAIT_EVENT_NUM) || 0 == App::EPOLL_WAIT_EVENT_NUM) {
		App::EPOLL_WAIT_EVENT_NUM = DefaultConfig::App::EPOLL_WAIT_EVENT_NUM;
	}

	if (conf->readInt(section, "node_group_num", &App::NODE_GROUP_NUM)
			|| 0 == App::NODE_GROUP_NUM) {
		App::NODE_GROUP_NUM = DefaultConfig::App::NODE_GROUP_NUM;
	}

	if (conf->readInt(section, "node_group_size", &App::NODE_GROUP_SIZE)
			|| 0 == App::NODE_GROUP_SIZE) {
		App::NODE_GROUP_SIZE = DefaultConfig::App::NODE_GROUP_SIZE;
	}

	App::TOTAL_SUPPORT_USER_NUM = App::NODE_GROUP_NUM * App::NODE_GROUP_SIZE;
	App::TOTAL_THREAD_NUM = App::NODE_GROUP_NUM + 4;

	if (conf->readInt(section, "message_max_num_in_queue",
			&App::MESSAGE_MAX_NUM_IN_QUEUE)
			|| 0 == App::MESSAGE_MAX_NUM_IN_QUEUE) {
		App::MESSAGE_MAX_NUM_IN_QUEUE = App::NODE_GROUP_SIZE * 8;
	}

	if (conf->readInt(section, "thread_stack_size", &App::THREAD_STACK_SIZE)
			|| 0 == App::THREAD_STACK_SIZE) {
		App::THREAD_STACK_SIZE = DefaultConfig::App::THREAD_STACK_SIZE;
	}

	if (conf->readInt(section, "heartbeat_interval", &App::HEARTBEAT_INTERVAL)
			|| 0 == App::HEARTBEAT_INTERVAL) {
		App::HEARTBEAT_INTERVAL = DefaultConfig::App::HEARTBEAT_INTERVAL;
	}
}

none_ initializeMsg(CConfReader *conf) {
	assert(null_v != conf);
	const c1_ *section = "msg";

	ub4_ validDuration;

	if (conf->readInt(section, "valid_duration", &validDuration)
			|| 0 == validDuration) {
		Msg::VALID_DURATION = DefaultConfig::Msg::VALID_DURATION * 60 * 60
				* 1000000;
	} else {
		Msg::VALID_DURATION = validDuration * 60 * 60 * 1000000;
	}

	if (conf->readByte(section, "valid_number", &Msg::VALID_NUMBER)
			|| 0 == Msg::VALID_NUMBER) {
		Msg::VALID_NUMBER = DefaultConfig::Msg::VALID_NUMBER;
	}
}

none_ initializeRedis(CConfReader *conf) {
	assert(null_v != conf);
	const c1_ *section = "redis";

	if (conf->readString(section, "host", Redis::HOST, Size::URL)
			|| 0 == Redis::HOST[0]) {
		strncpy(Redis::HOST, DefaultConfig::Redis::HOST, Size::URL);
	}

	if (conf->readShort(section, "port", &Redis::PORT) || 0 == Redis::PORT) {
		Redis::PORT = DefaultConfig::Redis::PORT;
	}

	if (conf->readInt(section, "timeout", &Redis::TIMEOUT)
			|| 0 == Redis::TIMEOUT) {
		Redis::TIMEOUT = DefaultConfig::Redis::TIMEOUT;
	}

	if (conf->readString(section, "auth", Redis::AUTH, Size::PASSWORD)
			|| 0 == Redis::AUTH[0]) {
		strncpy(Redis::AUTH, DefaultConfig::Redis::AUTH, Size::PASSWORD);
	}

	if (conf->readByte(section, "db", &Redis::DB)) {
		Redis::DB = DefaultConfig::Redis::DB;
	}
}

}
