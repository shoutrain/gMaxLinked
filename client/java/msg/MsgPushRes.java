package gMaxLinked.exmpale.android.msg;

import java.nio.ByteBuffer;

import gMaxLinked.exmpale.android.tool.NetworkDataHelper;

public class MsgPushRes extends MsgExtra implements MsgRes {

	public static final int MIN_SIZE = Header.SIZE + Extra.SIZE + 8;

	public MsgPushRes(Header header) {
		super(header);
	}

	@Override
	public boolean parseBody(ByteBuffer buffer) {
		if (header.headerSize - Header.SIZE > buffer.capacity()) {
			return false;
		}

		if (false == extra.parseExtra(buffer)) {
			return false;
		}

		bodySrc = NetworkDataHelper.getUInt(buffer);
		short srcNameSize = NetworkDataHelper.getUByte(buffer);
		bodyMsgType = NetworkDataHelper.getUByte(buffer);
		long msgId = NetworkDataHelper.getUInt(buffer);
		int msgIdExtra = NetworkDataHelper.getUShort(buffer);
		bodyMsgId = msgId + (((long) msgIdExtra) << 32);
		short msgTitleSize = NetworkDataHelper.getUByte(buffer);
		int msgContentSize = NetworkDataHelper.getUShort(buffer);
		bodyMsgAttachment = NetworkDataHelper.getUShort(buffer);
		bodyOccurTime = NetworkDataHelper.getUInt(buffer);
		bodySrcName = NetworkDataHelper.getString(buffer, srcNameSize);
		bodyMsgTitle = NetworkDataHelper.getString(buffer, msgTitleSize);
		bodyMsgContent = NetworkDataHelper.getString(buffer, msgContentSize);

		return true;
	}

	public long bodySrc; // unsigned int
	public String bodySrcName;
	public short bodyMsgType; // unsigned char
	public long bodyMsgId; // unsigned long, less than 0x8000000000000000
	public int bodyMsgAttachment; // unsigned short
	public long bodyOccurTime; // unsigned int
	public String bodyMsgTitle;
	public String bodyMsgContent;
}
