package com.exmaple.msg;

import java.nio.ByteBuffer;

import com.avcon.tool.NetworkDataHelper;

public final class Header implements MsgSegment {

	public Header() {
	}
	
	public Header(int type, int cmd, int ver, short lang, long seq, long stmp, long ext) {
		this.type = type;
		this.cmd = cmd;
		this.ver = ver;
		this.lang = lang;
		this.seq = seq;
		this.stmp = stmp;
		this.ext = ext;
	}
	
	public int segmentSize() {
		return 29;
	}

	public boolean parse(ByteBuffer buffer) {
		if (null == buffer || this.segmentSize() > buffer.capacity() - buffer.position()) {
			return false;
		}

		this.size = NetworkDataHelper.getUShort(buffer);
		this.type = NetworkDataHelper.getUShort(buffer);
		this.cmd = NetworkDataHelper.getUShort(buffer);
		this.ver = NetworkDataHelper.getUShort(buffer);
		this.lang = NetworkDataHelper.getUByte(buffer);
		this.seq = NetworkDataHelper.getUInt(buffer);
		this.stmp = NetworkDataHelper.getLong(buffer);
		this.ext = NetworkDataHelper.getLong(buffer);
		
		return true;
	}
	
	public boolean compose(ByteBuffer buffer) {
		if (null == buffer || this.segmentSize() > buffer.capacity() - buffer.position()) {
			return false;
		}

		NetworkDataHelper.setUShort(buffer, this.size);
		NetworkDataHelper.setUShort(buffer, this.type);
		NetworkDataHelper.setUShort(buffer, this.cmd);
		NetworkDataHelper.setUShort(buffer, this.ver);
		NetworkDataHelper.setUByte(buffer, this.lang);
		NetworkDataHelper.setUInt(buffer, this.seq);
		NetworkDataHelper.setLong(buffer, this.stmp);
		NetworkDataHelper.setLong(buffer, this.ext);

		return true;
	}

	// Fields in network message header
	public int size = 0; // unsigned short
	public int type = 0; // unsigned short
	public int cmd = 0; // unsigned short
	public int ver = 0; // unsigned short
	public short lang = 0; // unsigned char
	public long seq = 0; // unsigned long
	public long stmp = 0; // unsigned long
	public long ext = 0; // unsigned long

}
