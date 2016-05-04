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
const ub4_ MSG_FIXED_LENGTH = 128;

// MSG_FIXEDLENGTH - sizeof(THeader) - sizeof(TExtra)
const ub4_ MSG_BODY_MAX_LENGTH = 116;

////////////////////////////////////
// message bit operators
////////////////////////////////////
// message direction(the 12th bits):
//   0001 0000 0000 0000 - MSGACK
//   0000 XXXX 0000 0000 - message category
//   0000 0000 XXXX XXXX - message command id
const ub2_ MS_ACK = 0x1000;

////////////////////////////////////
// message command ids
////////////////////////////////////
// for network messages
const ub2_ MC_HANDSHAKE = 0x0001;
const ub2_ MC_REALTIME = 0x0101;

// for b4_ernal messages
const ub2_ MC_TIMER = 0x0f01;
const ub2_ MC_OVER = 0x0f02;

#pragma pack(1)

struct THeader {
	ub2_ length;
	ub2_ cmd;
};

struct TExtra {
	ub8_ transaction;
	ub4_ sequence;
};

struct TMsg {
	THeader header;
	TExtra extra;
	ub1_ body[MSG_BODY_MAX_LENGTH];
};

struct TMBAck {
	b4_ result;
};

struct TOnlyOnceFields {
	ub2_ softwareVersion;
};

struct TMBTimer {
	ub8_ timerId;
};

struct TMBOver {
	b4_ reason;
};

#pragma pack()
}

#endif // _MESSAGE_H_
