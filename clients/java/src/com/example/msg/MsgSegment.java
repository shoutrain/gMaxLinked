package com.exmaple.msg;

import java.nio.ByteBuffer;

public interface MsgSegment {
	
	int segmentSize();
	boolean parse(ByteBuffer buffer);
	boolean compose(ByteBuffer buffer);
	
}
