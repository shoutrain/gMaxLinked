package com.exmaple.msg;

import java.nio.ByteBuffer;

import com.avcon.conf.Size;
import com.avcon.tool.NetworkDataHelper;

public class MsgHandshake implements MsgSegment {

	public MsgHandshake(long build, String sessionId) {
		this.build = build;
		this.sessionId = sessionId;
	}
	
	@Override
	public int segmentSize() {
		return 4 + Size.SESSION_ID;
	}
	
	@Override
	public boolean parse(ByteBuffer buffer) {
		assert(false);
		return true;
	}
	
	@Override
	public boolean compose(ByteBuffer buffer) {
		if (null == buffer || segmentSize() > buffer.capacity() - buffer.position()) {
			return false;
		}
		
		NetworkDataHelper.setUInt(buffer, this.build);
		NetworkDataHelper.setString(buffer, this.sessionId);
		
		return true;
	}
	

	// Fields in network message
	public long build = 0; // unsigned int
	public String sessionId; // md5 with 16 length and 1 for '\0'
}
