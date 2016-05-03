/*
 ============================================================================
 Name        : Config.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Size.h"

namespace Config {

#ifndef GLOBAL_CONFIG
#define GLOBAL_CONFIG extern

namespace App {

GLOBAL_CONFIG char INI_FILE[Size::INI_FILE];
GLOBAL_CONFIG unsigned int RUN_AS_DAEMON;
GLOBAL_CONFIG char NAME[Size::NAME];
GLOBAL_CONFIG unsigned int BASE_BUILD;
GLOBAL_CONFIG char LISTEN_IP[Size::IP_V4];
GLOBAL_CONFIG unsigned short LISTEN_PORT;
GLOBAL_CONFIG unsigned int EPOLL_WAIT_EVENT_NUM;
GLOBAL_CONFIG unsigned int NODE_GROUP_NUM;
GLOBAL_CONFIG unsigned int NODE_GROUP_SIZE;
GLOBAL_CONFIG unsigned int TOTAL_SUPPORT_USER_NUM;
GLOBAL_CONFIG unsigned int TOTAL_THREAD_NUM;
GLOBAL_CONFIG unsigned int MESSAGE_MAX_NUM_IN_QUEUE;
GLOBAL_CONFIG unsigned int THREAD_STACK_SIZE;
GLOBAL_CONFIG unsigned int HANDSHAKE_INTERVAL;

}

namespace Redis {

GLOBAL_CONFIG char HOST[Size::URL];
GLOBAL_CONFIG unsigned short PORT;
GLOBAL_CONFIG unsigned int TIMEOUT; // Seconds

}

#endif

void initialize(const char *confFileName);

}

#endif // _CONFIG_H_
