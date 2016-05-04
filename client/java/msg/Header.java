package gMaxLinked.exmpale.android.msg;

import java.nio.ByteBuffer;

import gMaxLinked.exmpale.android.tool.NetworkDataHelper;

public final class Header {

	public static final int SIZE = 3;

	public Header() {
	}

	public Header(int size, short id) {
		assert (0 < size && 0xff >= size);
		assert (0 < id && 0xff >= id);
		headerSize = size;
		headerId = id;
	}

	protected boolean composeHeader(ByteBuffer buffer) {
		if (null == buffer || SIZE > buffer.capacity()) {
			return false;
		}

		NetworkDataHelper.setUShort(buffer, headerSize);
		NetworkDataHelper.setUByte(buffer, headerId);

		return true;
	}

	public MsgRes parseHeader(ByteBuffer buffer) {
		if (null == buffer || SIZE > buffer.capacity()) {
			return null;
		}

		headerSize = NetworkDataHelper.getUShort(buffer);
		headerId = NetworkDataHelper.getUByte(buffer);

		switch (headerId) {
		case MsgId.MSG_READY_RES:
			if (headerSize < MsgReadyRes.MIN_SIZE) {
				return null;
			}

			return new MsgReadyRes(this);
		case MsgId.MSG_HANDSHAKE_RES:
			if (headerSize != MsgHandshakeRes.MIN_SIZE) {
				return null;
			}

			return new MsgHandshakeRes(this);
		case MsgId.MSG_PUSH_RES:
			if (headerSize < MsgPushRes.MIN_SIZE) {
				return null;
			}

			return new MsgPushRes(this);
		}
		assert false;
		return null;
	}

	// Fields in network message header
	public int headerSize = 0; // unsigned short
	public short headerId = 0; // unsigned char
}
