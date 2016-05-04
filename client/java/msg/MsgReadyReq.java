package gMaxLinked.exmpale.android.msg;

import java.lang.reflect.Array;
import java.nio.ByteBuffer;

import gMaxLinked.exmpale.android.tool.NetworkDataHelper;

public class MsgReadyReq extends Msg implements MsgReq {

	public MsgReadyReq(long versionCode, long userId, String session) {
		super(9 + Array.getLength(session.getBytes()) + 1,
				MsgId.MSG_READY_REQ);

		assert (0 != userId);
		assert (null != session && 0 < session.length());

		bodyBuild = versionCode;
		bodyUserId = userId;
		bodySession = session;
	}

	@Override
	public byte[] getBytes() {

		ByteBuffer buffer = ByteBuffer.allocate(header.headerSize);

		if (false == header.composeHeader(buffer)) {
			return null;
		}

		NetworkDataHelper.setUInt(buffer, bodyBuild);
		NetworkDataHelper.setUInt(buffer, bodyUserId);

		int size = Array.getLength(bodySession.getBytes()) + 1;

		NetworkDataHelper.setUByte(buffer, (short) size);
		NetworkDataHelper.setString(buffer, bodySession);

		return buffer.array();
	}

	public long bodyBuild; // unsigned int
	public long bodyUserId; // unsigned int
	public String bodySession;
}
