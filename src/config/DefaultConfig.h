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

const char INI_FILE[] = "CollectHandler.conf";

const unsigned int RUN_AS_DAEMON = 0;

const char NAME[] = "CollectHandler";
const unsigned int BASE_BUILD = 13;

const char LISTEN_IP[] = "0.0.0.0";
const short LISTEN_PORT = 10505;

const unsigned int EPOLL_WAIT_EVENT_NUM = 16;

const unsigned int NODE_GROUP_NUM = 4;
const unsigned int NODE_GROUP_SIZE = 16;

// const unsigned int MESSAGE_MAX_NUM_IN_QUEUE = NODE_GROUP_SIZE;

const unsigned int THREAD_STACK_SIZE = 4096; // k

const unsigned int HANDSHAKE_INTERVAL = 10; // s

}

namespace Redis {

const char HOST[] = "localhost";
const short PORT = 6379;
const int TIMEOUT = 1; // s

}

}

#endif // _DEFAULT_CONFIG_H_
