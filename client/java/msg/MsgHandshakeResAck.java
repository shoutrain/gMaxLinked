package gMaxLinked.exmpale.android.msg;

import java.nio.ByteBuffer;

public class MsgHandshakeResAck extends Msg implements MsgReq {

	public MsgHandshakeResAck() {
		super((short)0, MsgId.MSG_HANDSHAKE_RES_ACK);
	}

	@Override
	public byte[] getBytes() {

		ByteBuffer buffer = ByteBuffer.allocate(header.headerSize);

		if (false == header.composeHeader(buffer)) {
			return null;
		}

		return buffer.array();
	}
}
