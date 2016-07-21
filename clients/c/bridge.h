#ifndef _bridge_h
#define _bridge_h

int net_worker_start(const char *ip, unsigned short port,
        unsigned int reconnect_interval, unsigned int build,
        const char *session_id);

void net_worker_stop();

int net_worker_send(unsigned char dstType, unsigned long long dstId,
        const char *json);

typedef unsigned long long (*ON_CONNECTED)();

typedef void (*ON_READY)();

typedef void (*ON_PUSH)(unsigned char ornType, unsigned long long ornId,
        unsigned long long ornExtId, unsigned long long messageId, const char *json,
        unsigned long long stmp);

typedef void (*ON_SEND_ACK)(unsigned long long seq, unsigned short code,
        unsigned long long messageId);

typedef void (*ON_CLOSED)();

typedef void (*ON_ERROR)(int err_code, const char *err_desc);

void set_on_connected(ON_CONNECTED on_connect);

void set_on_ready(ON_READY on_ready);

void set_on_push(ON_PUSH on_push);

void set_on_send_ack(ON_SEND_ACK on_send_ack);

void set_on_error(ON_ERROR on_error);

void set_on_closed(ON_CLOSED on_closed);

#endif /* _bridge_h */
