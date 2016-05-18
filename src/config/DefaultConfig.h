/*
 ============================================================================
 Name        : DefaultConfig.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _DEFAULT_CONFIG_H_
#define _DEFAULT_CONFIG_H_

#include "Size.h"

namespace DefaultConfig {

	namespace App {

		const c1_ CONF_FILE[] = "RealtimeServer.conf";

		const ub4_ RUN_AS_DAEMON = 0;

		const c1_ NAME[] = "RealtimeServer";
		const ub4_ BASE_BUILD = 0;

		const c1_ LISTEN_IP[] = "0.0.0.0";
		const ub2_ LISTEN_PORT = 10505;

		const ub4_ EPOLL_WAIT_EVENT_NUM = 16;

		const ub4_ NODE_GROUP_NUM = 4;
		const ub4_ NODE_GROUP_SIZE = 16;

// const ub4_ MESSAGE_MAX_NUM_IN_QUEUE = NODE_GROUP_SIZE;

		const ub4_ THREAD_STACK_SIZE = 4096;// k

		const ub4_ HEARTBEAT_INTERVAL = 10;// s

	}

	namespace Redis {

		const c1_ HOST[] = "localhost";
		const ub2_ PORT = 6379;
		const ub4_ TIMEOUT = 1; // s
		const c1_ AUTH[] = "123456";
		const ub1_ DB = 0;
	}

}

#endif // _DEFAULT_CONFIG_H_
