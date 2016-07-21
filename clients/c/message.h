#ifndef _message_h_
#define _message_h_

#include "size.h"

////////////////////////////////////
// type values in THeader
////////////////////////////////////
#define MT_CONTROL 0x0001
#define MT_ACCOUNT 0x0002
#define MT_SERVICE 0x0003

#define MT_SIGN_ACK 0x0010
#define MT_SIGN_LOG 0x0020
#define MT_SIGN_FEE 0x0040
#define MT_SIGN_GRP 0x0080

////////////////////////////////////
// cmd values in THeader
////////////////////////////////////
// network messages: 0xXXXX except 0x8XXX
#define MC_HAND_SHAKE 0x0001
#define MC_HEART_BEAT 0x0002

#define MC_SEND_MSG 0x0003
#define MC_PUSH_MSG 0x0004

////////////////////////////////////
// lang values in THeader
////////////////////////////////////
#define ML_CN 0x01
#define ML_TW 0x02
#define ML_EN 0x03

////////////////////////////////////
// PDUs
////////////////////////////////////
#pragma pack(1)

struct THeader {
    unsigned short size;
    unsigned short type;
    unsigned short cmd;
    unsigned short ver;
    unsigned char lang;
    unsigned int seq;
    unsigned long long stmp;
    unsigned long long ext;
};

struct TAck {
    unsigned short code;
};

struct TPDUHandShake {
    struct THeader header;
    unsigned int build;
    unsigned long long lastUpdate;
    char sessionId[SIZE_SESSION_ID];
};

struct TPDUHandShakeAck {
    struct THeader header;
    struct TAck ack;
};

struct TPDUHeartBeat {
    struct THeader header;
};

struct TPDUHeartBeatAck {
    struct THeader header;
    struct TAck ack;
};

struct TPDUSendMsg {
    struct THeader header;
    unsigned char dstType; // 1 - user, 2 - group
    unsigned long long dstId;
    char json[SIZE_JSON];
};

struct TPDUSendMsgAck {
    struct THeader header;
    struct TAck ack;
    unsigned long long messageId;
};

struct TPDUPushMsg {
    struct THeader header;
    unsigned char ornType; // 1 - user, 2- group
    unsigned long long ornId;
    unsigned long long ornExtId;
    unsigned long long messageId;
    char json[SIZE_JSON];
};

struct TPDUPushMsgAck {
    struct THeader header;
    struct TAck ack;
};

#pragma pack()

#endif // _message_h_
