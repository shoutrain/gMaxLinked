/*
 ============================================================================
 Name        : Message.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "../config/Size.h"

namespace Message {

// pdu with fixed size is easier to develop and has higher performance
const unsigned int MSG_FIXED_LENGTH = 128;

// MSG_FIXEDLENGTH - sizeof(THeader) - sizeof(TExtra)
const unsigned int MSG_BODY_MAX_LENGTH = 116;

////////////////////////////////////
// message bit operators
////////////////////////////////////
// message direction(the 12th bits):
//   0001 0000 0000 0000 - MSGACK
//   0000 XXXX 0000 0000 - message category
//   0000 0000 XXXX XXXX - message command id
const unsigned short MS_ACK = 0x1000;

////////////////////////////////////
// message command ids
////////////////////////////////////
// for network messages
const unsigned short MC_HANDSHAKE = 0x0001;
const unsigned short MC_REALTIME = 0x0101;

// for internal messages
const unsigned short MC_TIMER = 0x0f01;
const unsigned short MC_OVER = 0x0f02;

#pragma pack(1)

struct THeader {
	unsigned short length;
	unsigned short cmd;
};

struct TExtra {
	long unsigned int transaction;
	unsigned int sequence;
};

struct TMsg {
	THeader header;
	TExtra extra;
	unsigned char body[MSG_BODY_MAX_LENGTH];
};

struct TMBAck {
	int result;
};

struct TOnlyOnceFields {
	unsigned short softwareVersion;
};

struct TMBTimer {
	long unsigned int timerId;
};

struct TMBOver {
	int reason;
};

#pragma pack()
}

#endif // _MESSAGE_H_
