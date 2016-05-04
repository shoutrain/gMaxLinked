#ifndef _message_h_
#define _message_h_

#include "size.h"

// pdu with fixed size is easier to develop and has higher performance
#define MSG_FIXED_LENGTH 128

// MSG_FIXED_LENGTH - sizeof(struct msg_header) - sizeof(struct msg_extra)
#define MSG_BODY_MAX_LENGTH 112

extern const unsigned short MS_ACK;

extern const unsigned short MC_HANDSHAKE;
extern const unsigned short MC_REALTIME;

#pragma pack(1)

struct msg_header {
	unsigned short length;
	unsigned short cmd;
};

struct msg_extra {
	unsigned long long transaction;
	unsigned int sequence;
};

struct msg {
	struct msg_header header;
	struct msg_extra extra;
	unsigned char body[MSG_BODY_MAX_LENGTH];
};

struct mb_ack {
	int result;
};

struct mb_handshake {
	char version[VERSION_SIZE];
	unsigned int build;
};

#pragma pack()
// ----------------Share with CollectHandler ends----------------//

#endif // _message_h_
