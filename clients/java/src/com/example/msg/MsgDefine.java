package com.exmaple.msg;

public interface MsgDefine {

	int MSG_MAX_LENGTH = 512;

	////////////////////////////////////
	// type values in THeader
	////////////////////////////////////
	int MT_CONTROL = 0x0001; // unsigned short
	int MT_ACCOUNT = 0x0002; // unsigned short
	int MT_SERVICE = 0x0003; // unsigned short

	int MT_SIGN_ACK = 0x0010; // unsigned short
	int MT_SIGN_LOG = 0x0020; // unsigned short
	int MT_SIGN_FEE = 0x0040; // unsigned short

	////////////////////////////////////
	// cmd values in THeader
	////////////////////////////////////
	// network messages: 0xXXXX except 0x8XXX
	int MC_HAND_SHAKE = 0x0001; // unsigned short
	int MC_HEART_BEAT = 0x0002; // unsigned short

	////////////////////////////////////
	// lang values in THeader
	////////////////////////////////////
	short ML_CN = 0x01; // unsigned char
	short ML_TW = 0x02; // unsigned char
	short ML_EN = 0x03; // unsigned char

}
