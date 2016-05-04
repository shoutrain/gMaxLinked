#ifndef _connection_h_
#define _connection_h_

#include <stdbool.h>

struct connection_info;

int cnn_start(struct connection_info *ci, bool join);

void cnn_stop(struct connection_info *ci, bool sync);

int cnn_send(struct connection_info *ci, const unsigned char *buffer,
		unsigned int size);

unsigned int cnn_get_connection_num();

#endif // _connection_h_
