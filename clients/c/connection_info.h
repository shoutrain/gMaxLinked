#ifndef _connection_info_h_
#define _connection_info_h_

#include <pthread.h>

#include "size.h"

typedef void (*cnn_ON_CONNECTED)(void *parameter);

typedef void (*cnn_ON_RECEIVED)(void *parameter, const unsigned char *data,
        unsigned int size);

typedef void (*cnn_ON_ERROR)(void *parameter, int err_code,
        const char *err_desc);

typedef void (*cnn_ON_CLOSED)(void *parameter);

struct cnn_callback_funs {
    cnn_ON_CONNECTED on_connected;
    cnn_ON_RECEIVED on_received;
    cnn_ON_ERROR on_error;
    cnn_ON_CLOSED on_closed;
};

struct connection_info {
    void *parameter;

    char ip[SIZE_IP_V4];
    unsigned short port;
    unsigned int reconnect_interval; // seconds

    struct cnn_callback_funs callback_funs;

    // following parameters are managed by the application
    int socket;
    pthread_t tid;
    int is_running;
    int is_connected;
    int needs_reconnect;

    unsigned char msg_buf[MSG_MAX_LENGTH];
    unsigned int offset;
    unsigned int pending_size;
};

#endif // _connection_info_h_
