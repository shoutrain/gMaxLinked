package com.exmaple.msg;

import java.nio.ByteBuffer;

import com.avcon.tool.NetworkDataHelper;

public final class Ack implements MsgSegment {

	public Ack() {
	}

	public Ack(int code) {
		this.code = code;
	}
	
	public int segmentSize() {
		return 2;
	}

	public boolean parse(ByteBuffer buffer) {
		if (null == buffer || this.segmentSize() > buffer.capacity() - buffer.position()) {
			return false;
		}
		
		this.code = NetworkDataHelper.getUShort(buffer);
		
		return true;
	}
	
	public boolean compose(ByteBuffer buffer) {
		if (null == buffer || this.segmentSize() > buffer.capacity() - buffer.position()) {
			return false;
		}
		
		NetworkDataHelper.setUShort(buffer, this.code);
		
		return true;
	}

	// Fields in network message header
	public int code = 0; // unsigned short

}
