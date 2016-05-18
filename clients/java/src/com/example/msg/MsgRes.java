package com.exmaple.msg;

import java.nio.ByteBuffer;

public interface MsgRes {

	public boolean parseBody(ByteBuffer buffer);
}
