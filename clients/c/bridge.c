#include <stdio.h>
#include <string.h>
#include <time.h>

#include "connection_info.h"
#include "message.h"
#include "connection.h"
#include "bridge.h"

static struct connection_info _ci;
static bool _started = false;
static unsigned int _build = 0;
static char _session_id[SIZE_SESSION_ID];

static struct bridge_callback_funs {
    ON_CONNECTED on_connected;
    ON_READY on_ready;
    ON_PUSH on_push;
    ON_SEND_ACK on_send_ack;
    ON_ERROR on_error;
    ON_CLOSED on_closed;
} _bridge_callbacks;

static void _on_connected(void *parameter) {
    printf("on_connected\n");

    unsigned long long lastUpdate = 0;

    if (_bridge_callbacks.on_connected) {
        lastUpdate = _bridge_callbacks.on_connected();
    }

    struct TPDUHandShake msg;
    time_t t;

    msg.header.size = sizeof(struct TPDUHandShake);
    msg.header.type = MT_ACCOUNT;
    msg.header.cmd = MC_HAND_SHAKE;
    msg.header.ver = 0x0100;
    msg.header.lang = ML_CN;
    msg.header.seq = 0;
    time(&t);
    msg.header.stmp = t;
    msg.header.ext = 0;

    msg.build = _build;
    msg.lastUpdate = lastUpdate;
    strncpy(msg.sessionId, _session_id, SIZE_SESSION_ID - 1);
    msg.sessionId[16] = 0;

    if (0 != cnn_send(&_ci, (unsigned char *) &msg,
            sizeof(struct TPDUHandShake))) {
        printf("failed to send handshake msg\n");

        return;
    }
}

static void _on_received(void *parameter, const unsigned char *data,
        unsigned int size) {
    printf("on_received: %d bytes\n", size);

    struct THeader *header = (struct THeader *) data;

    if ((MT_ACCOUNT | MT_SIGN_ACK) == header->type
            && MC_HAND_SHAKE == header->cmd
            && sizeof(struct TPDUHandShakeAck) == size) {
        struct TPDUHandShakeAck *msg = (struct TPDUHandShakeAck *) data;
        printf("on_received: get TPDUHandShakeAck\n");

        if (msg->ack.code) {
            printf("on_received: failed to handshake with code-%d\n",
                    msg->ack.code);
        } else if (_bridge_callbacks.on_ready) {
            _bridge_callbacks.on_ready();
        }
    } else if (MT_CONTROL == header->type
            && MC_HEART_BEAT == header->cmd
            && sizeof(struct TPDUHeartBeat) == size) {
        printf("on_received: get TPDUHeartBeat\n");
        struct TPDUHeartBeatAck msg;
        time_t t;

        memcpy(&msg, data, sizeof(struct TPDUHeartBeat));
        msg.header.size = sizeof(struct TPDUHeartBeatAck);
        msg.header.type |= MT_SIGN_ACK;
        time(&t);
        msg.header.stmp = t;
        msg.ack.code = 0;

        if (0 != cnn_send(&_ci, (unsigned char *) &msg,
                sizeof(struct TPDUHeartBeatAck))) {
            printf("failed to send heartbeat ack msg\n");
        }
    } else if (MT_SERVICE == header->type
            && MC_PUSH_MSG == header->cmd
            && sizeof(struct TPDUPushMsg) == size) {
        struct TPDUPushMsg *msg = (struct TPDUPushMsg *) data;
        printf("on_received: get TPDUPushMsg\n");

        if (_bridge_callbacks.on_push) {
            _bridge_callbacks.on_push(msg->ornType, msg->ornId, msg->ornExtId,
                    msg->messageId, msg->json, msg->header.stmp);
        }

        struct TPDUPushMsgAck msgAck;
        time_t t;

        memcpy(&msgAck.header, &msg->header, sizeof(struct THeader));
        msgAck.header.size = sizeof(struct TPDUPushMsgAck);
        msgAck.header.type |= MT_SIGN_ACK;
        time(&t);
        msgAck.header.stmp = t;
        msgAck.ack.code = 0;

        if (0 != cnn_send(&_ci, (unsigned char *) &msgAck,
                sizeof(struct TPDUPushMsgAck))) {
            printf("failed to send push ack msg\n");
        }
    } else if ((MT_SERVICE | MT_SIGN_ACK) == header->type
            && MC_SEND_MSG == header->cmd
            && sizeof(struct TPDUSendMsgAck) == size) {
        struct TPDUSendMsgAck *msg = (struct TPDUSendMsgAck *) data;
        printf("on_received: get TPDUSendMsgAck\n");

        if (msg->ack.code) {
            printf("on_received: failed to send msg with code-%d\n",
                    msg->ack.code);
        } else {
            if (_bridge_callbacks.on_send_ack) {
                _bridge_callbacks.on_send_ack(msg->header.seq, msg->ack.code,
                        msg->messageId);
            }
        }
    } else {
        printf("on_received: get unknown messages\n");
    }
}

static void _on_error(void *parameter, int err_code, const char *err_desc) {
    printf("on_error: %d-%s\n", err_code, err_desc);

    if (_bridge_callbacks.on_error) {
        _bridge_callbacks.on_error(err_code, err_desc);
    }
}

static void _on_closed(void *parameter) {
    printf("on_closed\n");

    if (_bridge_callbacks.on_closed) {
        _bridge_callbacks.on_closed();
    }
}

int net_worker_start(const char *ip, unsigned short port,
        unsigned int reconnect_interval, unsigned int build,
        const char *session_id) {
    if (true == _started) {
        printf("the proccess has been started");

        return -1;
    }

    _started = true;

    memset(&_ci, 0, sizeof(struct connection_info));
    strncpy(_ci.ip, ip, strlen(ip));
    _ci.port = port;
    _ci.reconnect_interval = reconnect_interval;
    _ci.callback_funs.on_connected = _on_connected;
    _ci.callback_funs.on_received = _on_received;
    _ci.callback_funs.on_error = _on_error;
    _ci.callback_funs.on_closed = _on_closed;
    _build = build;
    memset(_session_id, 0, SIZE_SESSION_ID);
    strncpy(_session_id, session_id, SIZE_SESSION_ID - 1);

    return cnn_start(&_ci, true);
}

void net_worker_stop() {
    if (true == _started) {
        cnn_stop(&_ci, true);
        _started = false;
    }
}

int net_worker_send(unsigned char dstType, unsigned long long dstId,
        const char *json) {
    if (!json || 0 == json[0] || SIZE_JSON <= strlen(json)) {
        return 1;
    }

    if (1 != dstType && 2 != dstType) {
        return 2;
    }

    struct TPDUSendMsg msg;
    time_t t;

    memset(&msg, 0, sizeof(struct TPDUSendMsg));
    msg.header.size = sizeof(struct TPDUSendMsg);
    msg.header.type = MT_SERVICE;
    msg.header.cmd = MC_SEND_MSG;
    msg.header.ver = 0x0100;
    msg.header.lang = ML_CN;
    msg.header.seq = 0;
    time(&t);
    msg.header.stmp = t;
    msg.header.ext = 0;

    msg.dstType = dstType;
    msg.dstId = dstId;
    strncpy(msg.json, json, SIZE_JSON - 1);

    return cnn_send(&_ci, (unsigned char *) &msg, sizeof(struct TPDUSendMsg));
}

void set_on_connected(ON_CONNECTED on_connected) {
    _bridge_callbacks.on_connected = on_connected;
}

void set_on_ready(ON_READY on_ready) {
    _bridge_callbacks.on_ready = on_ready;
}

void set_on_push(ON_PUSH on_push) {
    _bridge_callbacks.on_push = on_push;
}

void set_on_send_ack(ON_SEND_ACK on_send_ack) {
    _bridge_callbacks.on_send_ack = on_send_ack;
}

void set_on_error(ON_ERROR on_error) {
    _bridge_callbacks.on_error = on_error;
}

void set_on_closed(ON_CLOSED on_closed) {
    _bridge_callbacks.on_closed = on_closed;
}
