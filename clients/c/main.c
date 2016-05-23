//
//  main.c
//  RealtimeCTester
//
//  Created by 顾笑群 on 16/5/19.
//  Copyright (c) 2016 Rafael Gu. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "connection_info.h"
#include "message.h"
#include "connection.h"

struct connection_info ci;

void on_connected(void *parameter) {
    printf("on_connected\n");

    struct TPDUHandShake msg;
    time_t t;

    msg.header.size = sizeof(struct TPDUHandShake);
    msg.header.type = MT_CONTROL;
    msg.header.cmd = MC_HAND_SHAKE;
    msg.header.ver = 0x0100;
    msg.header.lang = 1;
    msg.header.seq = 0;
    time(&t);
    msg.header.stmp = t;
    msg.header.ext = 0;

    msg.build = 0;
    strncpy(msg.sessionId, "ABCDEFGHIJKLMNOP", 16);
    msg.sessionId[16] = 0;

    if (0 != cnn_send(&ci, (unsigned char *) &msg, sizeof(struct TPDUHandShake))) {
        printf("failed to send handshake msg\n");
    }
}

void on_received(void *parameter, const unsigned char *data,
        unsigned int size) {
    printf("on_received: %d bytes\n", size);

    struct THeader *header = (struct THeader *) data;

    if ((MT_CONTROL | MT_SIGN_ACK) == header->type
            && MC_HAND_SHAKE == header->cmd
            && sizeof(struct TPDUHandShakeAck) == size) {
        struct TPDUHandShakeAck *msg = (struct TPDUHandShakeAck *) data;

        if (msg->ack.code) {
            printf("on_received: failed to handshake with code-%d\n",
                    msg->ack.code);
        }
    } else if (MT_CONTROL == header->type
            && MC_HEART_BEAT == header->cmd
            && sizeof(struct TPDUHeartBeat) == size) {

        struct TPDUHeartBeatAck msg;

        memcpy(&msg, data, sizeof(struct TPDUHeartBeat));
        msg.header.size  = sizeof(struct TPDUHeartBeatAck);
        msg.header.type |= MT_SIGN_ACK;
        msg.ack.code = 0;

        if (0 != cnn_send(&ci, (unsigned char *) &msg,
                sizeof(struct TPDUHeartBeatAck))) {
            printf("failed to send heartbeat ack msg\n");
        }
    } else {
        printf("on_received: get unknown messages\n");
    }

}

void on_error(void *parameter, int err_code, const char *err_desc) {
    printf("on_error: %d-%s\n", err_code, err_desc);
}

void on_closed(void *parameter) {
    printf("on_closed\n");
}

int main(int argc, const char *argv[]) {
    char serverIp[] = "172.16.246.136";

    memset(&ci, 0, sizeof(struct connection_info));
    strncpy(ci.ip, serverIp, sizeof(serverIp) - 1);
    ci.port = 10505;
    ci.reconnect_interval = 10;
    ci.callback_funs.on_connected = on_connected;
    ci.callback_funs.on_received = on_received;
    ci.callback_funs.on_error = on_error;
    ci.callback_funs.on_closed = on_closed;

    if (0 != cnn_start(&ci, true)) {
        printf("failed to start connection\n");
        cnn_stop(&ci, true);
    }

    return 0;
}