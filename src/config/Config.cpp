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

#include "../common/CIniReader.h"
#include "DefaultConfig.h"

#include <string.h>

namespace Config {

namespace App {

char INI_FILE[Size::INI_FILE];
unsigned int RUN_AS_DAEMON;
char NAME[Size::NAME];
unsigned int BASE_BUILD;
char LISTEN_IP[Size::IP_V4];
unsigned short LISTEN_PORT;
unsigned int EPOLL_WAIT_EVENT_NUM;
unsigned int NODE_GROUP_NUM;
unsigned int NODE_GROUP_SIZE;
unsigned int TOTAL_SUPPORT_USER_NUM;
unsigned int TOTAL_THREAD_NUM;
unsigned int MESSAGE_MAX_NUM_IN_QUEUE;
unsigned int THREAD_STACK_SIZE;
unsigned int HANDSHAKE_INTERVAL;

}

namespace Redis {

char HOST[Size::URL];
unsigned short PORT;
unsigned int TIMEOUT;

}

void initializeApp(CIniReader *conf);
void initializeRedis(CIniReader *conf);

void initialize(const char *confFileName) {
	if (NULL != confFileName && 0 < strlen(confFileName)
			&& Size::INI_FILE > strlen(confFileName)) {
		strncpy(App::INI_FILE, confFileName, Size::INI_FILE);
	} else {
		strncpy(App::INI_FILE, DefaultConfig::App::INI_FILE, Size::INI_FILE);
	}

	CIniReader *conf = new CIniReader(App::INI_FILE);

	initializeApp(conf);
	initializeRedis(conf);
}

void initializeApp(CIniReader *conf) {
	assert(NULL != conf);
	const char *section = "app";

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
	App::TOTAL_THREAD_NUM = App::NODE_GROUP_NUM + 3;

	if (conf->readInt(section, "message_max_num_in_queue",
			&App::MESSAGE_MAX_NUM_IN_QUEUE)
			|| 0 == App::MESSAGE_MAX_NUM_IN_QUEUE) {
		App::MESSAGE_MAX_NUM_IN_QUEUE = App::NODE_GROUP_SIZE;
	}

	if (conf->readInt(section, "thread_stack_size", &App::THREAD_STACK_SIZE)
			|| 0 == App::THREAD_STACK_SIZE) {
		App::THREAD_STACK_SIZE = DefaultConfig::App::THREAD_STACK_SIZE;
	}

	if (conf->readInt(section, "handshake_interval", &App::HANDSHAKE_INTERVAL)
			|| 0 == App::HANDSHAKE_INTERVAL) {
		App::HANDSHAKE_INTERVAL = DefaultConfig::App::HANDSHAKE_INTERVAL;
	}
}

void initializeRedis(CIniReader *conf) {
	assert(NULL != conf);
	const char *section = "redis";

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
}

}
