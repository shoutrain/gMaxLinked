package gMaxLinked.exmpale.android.msg;

import java.nio.ByteBuffer;

public class MsgPushResAck extends MsgExtra implements MsgReq {

	public MsgPushResAck(long extra1, long extra2) {
		super((short) 0, MsgId.MSG_PUSH_RES_ACK, extra1, extra2);
	}

	@Override
	public byte[] getBytes() {
		ByteBuffer buffer = ByteBuffer.allocate(header.headerSize);

		if (false == header.composeHeader(buffer)) {
			return null;
		}

		if (false == extra.composeExtra(buffer)) {
			return null;
		}

		return buffer.array();
	}
}
