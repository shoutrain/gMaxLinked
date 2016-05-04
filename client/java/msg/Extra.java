package gMaxLinked.exmpale.android.msg;

import java.nio.ByteBuffer;

import gMaxLinked.exmpale.android.tool.NetworkDataHelper;

public class Extra {

	public static final int SIZE = 8;

	public Extra() {

	}

	public Extra(long extra1, long extra2) {
		this.extra1 = extra1;
		this.extra2 = extra2;
		assert (0 <= this.extra1 && 0xffffffffl >= this.extra1);
		assert (0 <= this.extra2 && 0xffffffffl >= this.extra2);
	}

	protected boolean composeExtra(ByteBuffer buffer) {
		if (null == buffer || SIZE > buffer.capacity() - Header.SIZE) {
			return false;
		}

		NetworkDataHelper.setUInt(buffer, extra1);
		NetworkDataHelper.setUInt(buffer, extra2);

		return true;
	}

	public boolean parseExtra(ByteBuffer buffer) {

		if (null == buffer || SIZE > buffer.capacity()) {
			return false;
		}

		extra1 = NetworkDataHelper.getUInt(buffer);
		extra2 = NetworkDataHelper.getUInt(buffer);

		return true;
	}

	// Fields in network message header extra
	public long extra1 = 0; // unsigned int
	public long extra2 = 0; // unsigned int
}
