//
//  main.c
//  RealtimeCTester
//
//  Created by 顾笑群 on 16/5/19.
//  Copyright (c) 2016 Rafael Gu. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bridge.h"

static char _sessionId[17];
static unsigned long long _groupId;
static char _ip[16] = "127.0.0.1";
static unsigned short _port = 10505;
static unsigned int _intervalSeconds = 10;
static unsigned int _buildNum = 1;

static unsigned int _totalSendNum = 0;
static unsigned int _totalPushNum = 0;

unsigned long long on_connected() {
    printf("bridge: on_connect\n");

    return 0;
}

void on_ready() {
    printf("bridge: on_ready\n");

    if (net_worker_send(2, _groupId, "{\"ct\":2,\"mt\":1,\"uri\":\"http:\\/\\/192.168.7.55:3000\\/images\\/124\",\"dateString\":\"06-27 14:43\",\"incoming\":false}")) {
        printf("bridge: on_ready - failed to send message\n");
    } else {
        _totalSendNum++;
    }
}

void on_push(unsigned char ornType, unsigned long long ornId, unsigned long long ornExtId,
        unsigned long long messageId, const char *json, unsigned long long stmp) {
    _totalPushNum++;
    printf("bridge: on_push(%u)|ornType:%u|ornId:%llu|ornExtId:%llu|messageId:%llu|json:%s|stmp:%llu\n",
            _totalPushNum, ornType, ornId, ornExtId, messageId, json, stmp);

    if (_totalPushNum < _totalSendNum * 8) {
        return;
    }

    if (net_worker_send(2, _groupId, "{\"ct\":2,\"mt\":1,\"uri\":\"http:\\/\\/192.168.7.55:3000\\/images\\/124\",\"dateString\":\"06-27 14:43\",\"incoming\":false}")) {
        printf("bridge: on_push - failed to send message\n");
    }
}

void on_send_ack(unsigned long long seq, unsigned short code,
        unsigned long long messageId) {
    printf("bridge: on_send_ack|seq:%llu|code:%u|messageId:%llu\n",
            seq, code, messageId);

    if (0 == code) {
        _totalSendNum++;
    }
}

void on_closed() {
    printf("bridge: on_closed\n");
}

void on_error(int err_code, const char *err_desc) {
    printf("bridge: %d-%s", err_code, err_desc);
}

int main(int argc, const char *argv[]) {
    if (3 > argc || 7 < argc) {
        printf("Usage: %s <session id> <group id> [ip(%s)] [port(%u)] [interval seconds(%u)] [build number(%u)]\n",
                argv[0], _ip, _port, _intervalSeconds, _buildNum);

        return 1;
    }

    if (16 != strlen(argv[1])) {
        printf("session id is a string with length of 16\n");

        return 2;
    }

    memset(_sessionId, 0, 17);
    strncpy(_sessionId, argv[1], 16);
    _groupId = (unsigned long long) atoll(argv[2]);

    if (0 == _groupId) {
        printf("group id must be integer\n");

        return 2;
    }

    if (3 < argc) {
        if (0 == argv[3][0] || 15 < strlen(argv[3])) {
            printf("%s is invalid ip\n", argv[3]);

            return 4;
        }

        memset(_ip, 0, 16);
        strncpy(_ip, argv[3], 15);
    }

    if (4 < argc) {
        _port = (unsigned short) atoi(argv[4]);

        if (1024 >= _port) {
            printf("%s is invalid port\n", argv[4]);
        }
    }

    if (5 < argc) {
        _intervalSeconds = (unsigned int) atoi(argv[5]);

        if (0 == _intervalSeconds) {
            printf("%s is invalid interval seconds\n", argv[5]);
        }
    }

    if (6 < argc) {
        _buildNum = (unsigned int) atoi(argv[6]);

        if (0 == _buildNum) {
            printf("%s is invalid build num\n", argv[6]);
        }
    }

    set_on_connected(on_connected);
    set_on_ready(on_ready);
    set_on_push(on_push);
    set_on_send_ack(on_send_ack);
    set_on_error(on_error);
    set_on_closed(on_closed);
    net_worker_start(_ip, _port, _intervalSeconds, _buildNum, _sessionId);

    return 0;
}
