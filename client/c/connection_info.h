#ifndef _connection_info_h_
#define _connection_info_h_

#include "size.h"

#include <pthread.h>

typedef int (*cnn_ON_CONNECTED)(unsigned int id);

typedef int (*cnn_ON_RECEIVED)(unsigned int id, const unsigned char *data,
		unsigned int size);

typedef int (*cnn_ON_ERROR)(unsigned int id, int err_code,
		const char *err_desc);

typedef int (*cnn_ON_CLOSED)(unsigned int id);

struct cnn_callback_funs {
	cnn_ON_CONNECTED on_connected;
	cnn_ON_RECEIVED on_received;
	cnn_ON_ERROR on_error;
	cnn_ON_CLOSED on_closed;
};

struct connection_info {
	unsigned int id;
	char name[CNN_NAME_SIZE];

	char ip[IP_V4_SIZE];
	unsigned short port;
	unsigned int heartbeat_interval;
	unsigned int reconnect_interval;

	struct cnn_callback_funs callback_funs;

	// following parameters are managed by the application
	int epoll_container;
	int socket;
	pthread_t tid;
	int is_running;
	int is_connected;
	int needs_reconnect;
};

#endif // _connection_info_h_
