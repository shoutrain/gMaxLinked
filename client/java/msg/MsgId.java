package gMaxLinked.exmpale.android.msg;

// All are based on unsigned int
public interface MsgId {

	// //////////////////////////////////
	// message bit operators
	// //////////////////////////////////
	// Message direction(the 4th&5th bits):
	// 00 - MSG_REQ
	// 01 - MSG_RES
	// 10 - MSG_REQ_ACK(never used)
	// 11 - MSG_RES_ACK
	short MSG_OPT_RES = 0x01 << 4; // unsigned char
	short MSG_OPT_ACK = 0x01 << 5; // unsigned char

	// Extra field(the 6th bit):
	// 0 - header with extra data
	// 1 - header without extra data
	short MSG_OPT_EXTRA = 0x01 << 6; // unsigned char

	// Message type(the 7th bit):
	// 0 - network message
	// 1 - interval message
	short MSG_OPT_INTERNAL = 0x01 << 7; // unsigned char

	// the 16th bit, reserved

	// //////////////////////////////////
	// message IDs
	// //////////////////////////////////
	// network messages
	short MSG_READY_REQ = 0x01; // unsigned char
	short MSG_READY_RES = MSG_OPT_RES | MSG_READY_REQ; // unsigned char

	short MSG_HANDSHAKE_RES = MSG_OPT_RES | 0x02; // unsigned char
	short MSG_HANDSHAKE_RES_ACK = MSG_OPT_ACK | MSG_HANDSHAKE_RES; // unsigned
																	// char

	short MSG_PUSH_RES = MSG_OPT_RES | MSG_OPT_EXTRA | 0x03; // unsigned char
	short MSG_PUSH_RES_ACK = MSG_OPT_ACK | MSG_PUSH_RES; // unsigned char
}
