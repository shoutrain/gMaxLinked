package gMaxLinked.exmpale.android.msg;

import java.nio.ByteBuffer;

import gMaxLinked.exmpale.android.tool.NetworkDataHelper;

public class MsgReadyRes extends Msg implements MsgRes {

	public static final int MIN_SIZE = Header.SIZE + 1;

	public MsgReadyRes(Header header) {
		super(header);
	}

	@Override
	public boolean parseBody(ByteBuffer buffer) {
		if (header.headerSize - Header.SIZE > buffer.capacity()) {
			return false;
		}

		bodyStatus = NetworkDataHelper.getUByte(buffer);

		return true;
	}

	public short bodyStatus = 0; // unsigned byte
}
