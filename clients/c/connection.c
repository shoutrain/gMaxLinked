#include "../../clients/c/connection.h"

#include <linux/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "../../clients/c/common.h"
#include "../../clients/c/connection_info.h"

#define EPOLL_EVENT_NUM     32
#define RECEIVE_BUFFER_SIZE 4096

static pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned int _connection_num;

#define __lock pthread_mutex_lock(&_lock)
#define __unlock pthread_mutex_unlock(&_lock)

static void _set_socket_nonblocking(int socket) {
	int opts = fcntl(socket, F_GETFL);
	int opt = 1;

	if (0 > opts) {
		log_fatal("failed to call fcntl with F_GETFL");
		exit(1);
	}

	opts |= O_NONBLOCK;

	if (0 > fcntl(socket, F_SETFL, opts)) {
		log_fatal("failed to call fctntl with F_SETFL");
		exit(1);
	}

	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

static void _init_socket(struct connection_info *ci) {
	ci->socket = socket(PF_INET, SOCK_STREAM, 0);

	if (-1 == ci->socket) {
		log_fatal("failed to create socket");
		exit(1);
	}

	_set_socket_nonblocking(ci->socket);

	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
	client_addr.sin_port = htons(0);

	if (bind(ci->socket, (struct sockaddr *) &client_addr,
			sizeof(client_addr))) {
		log_fatal("failed to bind socket");
		exit(1);
	}

	ci->epoll_container = epoll_create(EPOLL_EVENT_NUM);

	if (-1 == ci->epoll_container) {
		log_fatal("failed to call epoll_create");
		exit(1);
	}

	struct epoll_event ev;

	ev.events = EPOLLRDHUP | EPOLLOUT | EPOLLIN | EPOLLET;
	ev.data.fd = ci->socket;

	if (-1 == epoll_ctl(ci->epoll_container, EPOLL_CTL_ADD, ci->socket, &ev)) {
		log_fatal("failed to call epoll_ctl with EPOLL_CTL_ADD");
		exit(1);
	}
}

static void _fini_socket(struct connection_info *ci) {
	epoll_ctl(ci->epoll_container, EPOLL_CTL_DEL, ci->socket, 0);
	close(ci->socket);
	close(ci->epoll_container);
}

static void *_working(void *arg) {
	pthread_detach(pthread_self());

	__lock;
	_connection_num++;
	__unlock;

	struct connection_info *ci = (struct connection_info *) arg;
	ci->is_connected = 0;

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(ci->port);

	if (0 == inet_aton(ci->ip, &server_addr.sin_addr)) {
		log_fatal("failed to initialize socket ip");
		exit(1);
	}

	while (ci->is_running) {
		int ret = connect(ci->socket, (struct sockaddr *) &server_addr,
				sizeof(server_addr));

		if (0 == ret) {
			ci->callback_funs.on_connected(ci->id);
			ci->is_connected = 1;
		} else if (0 > ret && EINPROGRESS != errno) {
			log_info("failed to connect to destination");
			goo_sleep(ci->reconnect_interval, 0);
			_fini_socket(ci);
			_init_socket(ci);

			continue;
		}

		ci->needs_reconnect = 0;
		struct epoll_event events[EPOLL_EVENT_NUM];
		int ret_fds = 0;

		while (ci->is_running && !ci->needs_reconnect) {
			ret_fds = epoll_wait(ci->epoll_container, events, EPOLL_EVENT_NUM,
					ci->reconnect_interval);

			if (-1 == ret_fds) {
				switch (errno) {
				case EBADF:
					log_fatal("failed to call epoll_wait - EBADF");
					exit(1);
				case EFAULT:
					log_fatal("failed to call epoll_wait - EFAULT");
					exit(1);
				case EINTR:
					continue;
				case EINVAL:
					log_fatal("failed to call epoll_wait - EINVAL");
					exit(1);
				default:
					log_fatal("failed to call epoll_wait - Unknown");
					exit(1);
				}
			}

			int i = 0;

			for (; i < ret_fds; i++) {
				if ((events[i].events & EPOLLERR) && EINPROGRESS != errno) {
					ci->is_connected = 0;
					ci->needs_reconnect = 1;
					ci->callback_funs.on_error(ci->id, errno, strerror(errno));
					goo_sleep(ci->reconnect_interval, 0);
					_fini_socket(ci);
					_init_socket(ci);
					break;
				}

				if (events[i].events & EPOLLRDHUP) {
					ci->is_connected = 0;
					ci->needs_reconnect = 1;
					ci->callback_funs.on_closed(ci->id);
					goo_sleep(ci->reconnect_interval, 0);
					_fini_socket(ci);
					_init_socket(ci);
					break;
				}

				if (events[i].events & EPOLLOUT) {
					if (!ci->is_connected) {
						ci->is_connected = 1;
						ci->callback_funs.on_connected(ci->id);
						continue;
					}
				}

				if (events[i].events & EPOLLIN) {
					unsigned char buffer[RECEIVE_BUFFER_SIZE];
					int offset = 0;

					for (;;) {
						unsigned char tmp_buf[512];
						int len = recv(events[i].data.fd, tmp_buf, 512, 0);

						if (0 < len) { // read data from socket buffer
							if (len + offset > RECEIVE_BUFFER_SIZE) {
								log_fatal("the socket receiving buffer is too "
										"small");
								exit(1);
							}

							memcpy(buffer + offset, tmp_buf, len);
							offset += len;
						} else if (0 > len) {
							if (EAGAIN == errno || EWOULDBLOCK == errno) {
								// socket read buffer is empty
								if (0 < offset) {
									buffer[offset] = 0;
									ci->callback_funs.on_received(ci->id,
											buffer, offset);
									offset = 0;
								}
							} else {
								// error happened
							}

							break;
						} else {
							// the socket has been closed
							break;
						}
					}
				}
			}
		}
	}

	ci->is_connected = 0;
	ci->callback_funs.on_closed(ci->id);
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
		log_fatal("failed to create thread");

		return 1;
	}

	if (join) {
		if (pthread_join(ci->tid, NULL)) {
			log_fatal("failed to wait the thread");

			return 2;
		}
	}

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
			}

			offset += n;
		} while (size > offset);

		return 0;
	}

	return -1;
}

unsigned int cnn_get_connection_num() {
	return _connection_num;
}
