#include "connection.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "common.h"
#include "connection_info.h"
#include "message.h"

static pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned int _connection_num;

#define __lock pthread_mutex_lock(&_lock)
#define __unlock pthread_mutex_unlock(&_lock)

static void _set_socket_nonblocking(int socket) {
    int opts = fcntl(socket, F_GETFL);
    int opt = 1;

    if (0 > opts) {
        printf("failed to call fcntl with F_GETFL\n");
        exit(1);
    }

    opts |= O_NONBLOCK;

    if (0 > fcntl(socket, F_SETFL, opts)) {
        printf("failed to call fcntl with F_SETFL\n");
        exit(1);
    }

    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

static void _init_socket(struct connection_info *ci) {
    ci->socket = socket(PF_INET, SOCK_STREAM, 0);

    if (-1 == ci->socket) {
        printf("failed to create client socket\n");
        exit(1);
    }

    _set_socket_nonblocking(ci->socket);

    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(struct sockaddr_in));
    client_addr.sin_len = sizeof(struct sockaddr_in);
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);

    if (bind(ci->socket, (struct sockaddr *) &client_addr,
            sizeof(struct sockaddr_in))) {
        printf("failed to bind socket\n");
        exit(1);
    }
}

static void _fini_socket(struct connection_info *ci) {
    close(ci->socket);
}

static void *_working(void *arg) {
    __lock;
    _connection_num++;
    __unlock;

    struct connection_info *ci = (struct connection_info *) arg;
    ci->is_connected = 0;

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_len = sizeof(struct sockaddr_in);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ci->port);

    if (0 == inet_aton(ci->ip, &server_addr.sin_addr)) {
        printf("failed to initialize socket ip\n");
        exit(1);
    }

    while (ci->is_running) {
        int ret = connect(ci->socket, (struct sockaddr *) &server_addr,
                sizeof(struct sockaddr_in));

        if (0 == ret) {
            ci->callback_funs.on_connected(ci->parameter);
            ci->is_connected = 1;
        } else if (0 > ret && EINPROGRESS != errno) {
            printf("failed to connect to destination\n");
            _fini_socket(ci);
            goo_sleep(ci->reconnect_interval, 0);
            _init_socket(ci);

            continue;
        }

        ci->needs_reconnect = 0;
        fd_set fd_read_set;
        fd_set fd_write_set;
        fd_set fd_error_set;
        struct timeval tv;
        tv.tv_sec = ci->reconnect_interval;
        tv.tv_usec = 0;

        while (ci->is_running && !ci->needs_reconnect) {
            FD_ZERO(&fd_read_set);
            FD_ZERO(&fd_write_set);
            FD_ZERO(&fd_error_set);
            FD_SET(ci->socket, &fd_read_set);
            FD_SET(ci->socket, &fd_write_set);
            FD_SET(ci->socket, &fd_error_set);

            int ret = select(ci->socket + 1, &fd_read_set, &fd_write_set,
                    &fd_error_set, &tv);

            if (0 > ret) {
                ci->is_connected = 0;
                ci->needs_reconnect = 1;
                ci->callback_funs.on_closed(ci->parameter);
                _fini_socket(ci);
                goo_sleep(ci->reconnect_interval, 0);
                _init_socket(ci);

                break;
            } else if (0 == ret) {
                continue;
            }


            if (FD_ISSET(ci->socket, &fd_error_set)) {
                ci->is_connected = 0;
                ci->needs_reconnect = 1;
                ci->callback_funs.on_error(&ci, 1, "found error on the socket "
                        "after select\n");
                _fini_socket(ci);
                goo_sleep(ci->reconnect_interval, 0);
                _init_socket(ci);

                break;
            }

            if (FD_ISSET(ci->socket, &fd_write_set)) {
                if (0 == ci->is_connected) {
                    int status;
                    socklen_t slen = sizeof(int);

                    getsockopt(ci->socket, SOL_SOCKET, SO_ERROR,
                            (void *) &status, &slen);

                    if (0 == status) {
                        ci->is_connected = 1;
                        ci->callback_funs.on_connected(&ci);
                    } else {
                        break;
                    }
                }
            }

            if (FD_ISSET(ci->socket, &fd_read_set)) {
                for (; ;) {
                    if (0 == ci->pending_size) {
                        ci->pending_size = sizeof(struct THeader);
                    }

                    ssize_t len = recv(ci->socket, ci->msg_buf + ci->offset,
                            ci->pending_size - ci->offset, 0);

                    if (0 < len) { // read data from socket buffer
                        ci->offset += len;

                        if (sizeof(struct THeader) == ci->offset) {
                            struct THeader *header = (struct THeader *)ci->msg_buf;

                            if (sizeof(struct THeader) == header->size) {
                                ci->callback_funs.on_received(ci->parameter,
                                        ci->msg_buf, ci->offset);
                                ci->offset = 0;
                                ci->pending_size = 0;
                            } else {
                                ci->pending_size = header->size;
                            }
                        } else if (ci->pending_size == ci->offset){
                            ci->callback_funs.on_received(ci->parameter,
                                    ci->msg_buf, ci->offset);
                            ci->offset = 0;
                            ci->pending_size = 0;
                        }
                    } else if (0 > len) {
                        if (EAGAIN != errno || EWOULDBLOCK != errno) {
                            // error happened
                            // the socket has been closed
                            ci->is_connected = 0;
                            ci->needs_reconnect = 1;
                            ci->callback_funs.on_closed(ci->parameter);
                            _fini_socket(ci);
                            goo_sleep(ci->reconnect_interval, 0);
                            _init_socket(ci);
                        }

                        break;
                    } else {
                        // the socket has been closed
                        ci->is_connected = 0;
                        ci->needs_reconnect = 1;
                        ci->callback_funs.on_closed(ci->parameter);
                        _fini_socket(ci);
                        goo_sleep(ci->reconnect_interval, 0);
                        _init_socket(ci);

                        break;
                    }
                }
            }
        }
    }

    ci->is_connected = 0;
    ci->callback_funs.on_closed(ci->parameter);
    _fini_socket(ci);

    __lock;
    _connection_num--;
    __unlock;

    ci->tid = 0;

    return NULL;
}

int cnn_start(struct connection_info *ci, bool join) {
    _init_socket(ci);

    ci->is_running = 1;

    if (0 != pthread_create(&ci->tid, NULL, _working, ci)) {
        printf("failed to create thread\n");

        return 1;
    }

    if (join) {
        if (pthread_join(ci->tid, NULL)) {
            printf("failed to wait the thread\n");

            return 2;
        }
    } else {
        pthread_detach(ci->tid);
    };

    return 0;
}

void cnn_stop(struct connection_info *ci, bool sync) {
    ci->is_running = 0;

    if (sync) {
        while (ci->tid) {
            goo_sleep(0, 100);
        }
    }
}

int cnn_send(struct connection_info *ci, const unsigned char *buffer,
        unsigned int size) {
    assert(NULL != buffer);
    assert(0 < size);

    if (ci->is_connected) {
        ssize_t n = 0;
        unsigned int offset = 0;

        do {
            n = write(ci->socket, buffer + offset, size - offset);

            if (-1 == n) {
                if (EAGAIN == errno || EWOULDBLOCK == errno) {
                    goo_sleep(0, 10);
                    continue;
                } else {
                    return -2;
                }
            } else if (0 == n) {
                // the socket has been closed
                return -2;
            }

            offset += n;
        } while (size > offset);

        printf("sent %d bytes to server\n", size);

        return 0;
    }

    return -1;
}

unsigned int cnn_get_connection_num() {
    return _connection_num;
}
