package com.exmaple.msg;

import java.nio.ByteBuffer;

public class MsgHelper {
	
	public static Header parseHeader(ByteBuffer bufferMsg) {
		Header header = new Header();
		
		if (header.parse(bufferMsg)) {
			return header;
		}
		
		return null;
	}
	
	public static Ack parseAck(ByteBuffer bufferMsg) {
		Ack ack = new Ack();
		
		if (ack.parse(bufferMsg)) {
			return ack;
		}
		
		return null;
	}

	public static byte[] compose(Header header, Ack ack, MsgSegment body) {
		header.size = header.segmentSize();
		
		if (null != ack) {
			header.type |= MsgDefine.MT_SIGN_ACK;
			header.size += ack.segmentSize();
		}
		
		if (null != body) {
			header.size += body.segmentSize();
		}

		ByteBuffer buffer = ByteBuffer.allocate(header.size);
		
		if (!header.compose(buffer)) {
			return null;
		}
		
		if (null != ack) {
			if (!ack.compose(buffer)) {
				return null;
			}
		}
		
		if (null != body) {
			if (!body.compose(buffer)) {
				return null;
			}
		}
		
		return buffer.array();
	}
}
