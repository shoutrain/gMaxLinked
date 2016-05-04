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

		const c1_ INI_FILE[] = "gMaxLinked.conf";

		const ub4_ RUN_AS_DAEMON = 0;

		const c1_ NAME[] = "gMaxLinked";
		const ub4_ BASE_BUILD = 13;

		const c1_ LISTEN_IP[] = "0.0.0.0";
		const b2_ LISTEN_PORT = 10505;

		const ub4_ EPOLL_WAIT_EVENT_NUM = 16;

		const ub4_ NODE_GROUP_NUM = 4;
		const ub4_ NODE_GROUP_SIZE = 16;

// const ub4_ MESSAGE_MAX_NUM_IN_QUEUE = NODE_GROUP_SIZE;

		const ub4_ THREAD_STACK_SIZE = 4096;// k

		const ub4_ HANDSHAKE_INTERVAL = 10;// s

	}

	namespace Redis {

		const c1_ HOST[] = "localhost";
		const b2_ PORT = 6379;
		const b4_ TIMEOUT = 1; // s

	}

}

#endif // _DEFAULT_CONFIG_H_
