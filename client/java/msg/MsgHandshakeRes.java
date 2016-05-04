package gMaxLinked.exmpale.android.msg;

import java.nio.ByteBuffer;

public class MsgHandshakeRes extends Msg implements MsgRes {

	public static final int MIN_SIZE = Header.SIZE;

	public MsgHandshakeRes(Header header) {

		super(header);
	}

	@Override
	public boolean parseBody(ByteBuffer buffer) {

		return true;
	}
}
