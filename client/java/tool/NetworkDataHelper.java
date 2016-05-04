package gMaxLinked.exmpale.android.tool;

import java.lang.reflect.Array;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;

public class NetworkDataHelper {

	public static byte getSByte(ByteBuffer buffer) {

		assert (buffer.capacity() - buffer.position() >= 1);

		return buffer.get();
	}

	public static short getUByte(ByteBuffer buffer) {

		assert (buffer.capacity() - buffer.position() >= 1);

		return (short) (buffer.get() & 0x000000ff);
	}

	public static short getSShort(ByteBuffer buffer) {

		assert (buffer.capacity() - buffer.position() >= 2);

		byte[] data = new byte[2];

		data[1] = buffer.get();
		data[0] = buffer.get();

		return (short) (((data[0] << 8) & 0x0000ff00) | (data[1] & 0x000000ff));
	}

	public static int getUShort(ByteBuffer buffer) {

		assert (buffer.capacity() - buffer.position() >= 2);

		byte[] data = new byte[2];

		data[1] = buffer.get();
		data[0] = buffer.get();

		return ((data[0] << 8) & 0x0000ff00) | (data[1] & 0x000000ff);
	}

	public static int getSInt(ByteBuffer buffer) {

		assert (buffer.capacity() - buffer.position() >= 4);

		byte[] data = new byte[4];

		data[3] = buffer.get();
		data[2] = buffer.get();
		data[1] = buffer.get();
		data[0] = buffer.get();

		return ((data[0] << 24) & 0xff000000) | ((data[1] << 16) & 0x00ff0000)
				| ((data[2] << 8) & 0x0000ff00) | (data[3] & 0x000000ff);
	}

	public static long getUInt(ByteBuffer buffer) {

		assert (buffer.capacity() - buffer.position() >= 4);

		byte[] data = new byte[4];

		data[3] = buffer.get();
		data[2] = buffer.get();
		data[1] = buffer.get();
		data[0] = buffer.get();

		return ((data[0] << 24) & 0xff000000l)
				| ((data[1] << 16) & 0x00ff0000l)
				| ((data[2] << 8) & 0x0000ff00l) | (data[3] & 0x000000ffl);
	}

	public static long getLong(ByteBuffer buffer) {

		assert (buffer.capacity() - buffer.position() >= 8);

		long[] data = new long[8];

		data[7] = buffer.get();
		data[6] = buffer.get();
		data[5] = buffer.get();
		data[4] = buffer.get();
		data[3] = buffer.get();
		data[2] = buffer.get();
		data[1] = buffer.get();
		data[0] = buffer.get();

		return ((data[0] << 56) & 0xff00000000000000l)
				| ((data[1] << 48) & 0x00ff000000000000l)
				| ((data[2] << 40) & 0x0000ff0000000000l)
				| ((data[3] << 32) & 0x000000ff00000000l)
				| ((data[4] << 24) & 0x00000000ff000000l)
				| ((data[5] << 16) & 0x0000000000ff0000l)
				| ((data[6] << 8) & 0x000000000000ff00l)
				| (data[7] & 0x00000000000000ffl);
	}

	public static float getFloat(ByteBuffer buffer) {

		assert (buffer.capacity() - buffer.position() >= 4);

		byte[] data = new byte[4];

		data[3] = buffer.get();
		data[2] = buffer.get();
		data[1] = buffer.get();
		data[0] = buffer.get();

		return Float.intBitsToFloat(((data[0] << 24) & 0xff000000)
				| ((data[1] << 16) & 0x00ff0000)
				| ((data[2] << 8) & 0x0000ff00) | (data[3] & 0x000000ff));

	}

	public static double getDouble(ByteBuffer buffer) {

		assert (buffer.capacity() - buffer.position() >= 8);

		long[] data = new long[8];

		data[7] = buffer.get();
		data[6] = buffer.get();
		data[5] = buffer.get();
		data[4] = buffer.get();
		data[3] = buffer.get();
		data[2] = buffer.get();
		data[1] = buffer.get();
		data[0] = buffer.get();

		return Double.longBitsToDouble(((data[0] << 56) & 0xff00000000000000l)
				| ((data[1] << 48) & 0x00ff000000000000l)
				| ((data[2] << 40) & 0x0000ff0000000000l)
				| ((data[3] << 32) & 0x000000ff00000000l)
				| ((data[4] << 24) & 0x00000000ff000000l)
				| ((data[5] << 16) & 0x0000000000ff0000l)
				| ((data[6] << 8) & 0x000000000000ff00l)
				| (data[7] & 0x00000000000000ffl));
	}

	public static void setSByte(ByteBuffer buffer, byte data) {

		assert (buffer.capacity() - buffer.position() >= 1);

		buffer.put(data);
	}

	public static void setUByte(ByteBuffer buffer, short data) {

		assert (buffer.capacity() - buffer.position() >= 1);

		buffer.put((byte) (data & 0x000000ff));
	}

	public static void setSShort(ByteBuffer buffer, short data) {

		assert (buffer.capacity() - buffer.position() >= 2);

		buffer.put((byte) (data & 0x000000ff));
		buffer.put((byte) ((data & 0x0000ff00) >> 8));
	}

	public static void setUShort(ByteBuffer buffer, int data) {

		assert (buffer.capacity() - buffer.position() >= 2);

		buffer.put((byte) (data & 0x000000ff));
		buffer.put((byte) ((data & 0x0000ff00) >> 8));
	}

	public static void setSInt(ByteBuffer buffer, int data) {

		assert (buffer.capacity() - buffer.position() >= 4);

		buffer.put((byte) (data & 0x000000ffl));
		buffer.put((byte) ((data & 0x0000ff00l) >> 8));
		buffer.put((byte) ((data & 0x00ff0000l) >> 16));
		buffer.put((byte) ((data & 0xff000000l) >> 24));
	}

	public static void setUInt(ByteBuffer buffer, long data) {

		assert (buffer.capacity() - buffer.position() >= 4);

		buffer.put((byte) (data & 0x000000ffl));
		buffer.put((byte) ((data & 0x0000ff00l) >> 8));
		buffer.put((byte) ((data & 0x00ff0000l) >> 16));
		buffer.put((byte) ((data & 0xff000000l) >> 24));
	}

	public static void setLong(ByteBuffer buffer, long data) {

		assert (buffer.capacity() - buffer.position() >= 8);

		buffer.put((byte) (data & 0x00000000000000ffl));
		buffer.put((byte) ((data & 0x000000000000ff00l) >>> 8));
		buffer.put((byte) ((data & 0x0000000000ff0000l) >>> 16));
		buffer.put((byte) ((data & 0x00000000ff000000l) >>> 24));
		buffer.put((byte) ((data & 0x000000ff00000000l) >>> 32));
		buffer.put((byte) ((data & 0x0000ff0000000000l) >>> 40));
		buffer.put((byte) ((data & 0x00ff000000000000l) >>> 48));
		buffer.put((byte) ((data & 0xff00000000000000l) >>> 56));
	}

	public static void setFloat(ByteBuffer buffer, float data) {

		assert (buffer.capacity() - buffer.position() >= 4);

		buffer.put((byte) (Float.floatToIntBits(data) & 0x000000ff));
		buffer.put((byte) ((Float.floatToIntBits(data) & 0x0000ff00) >>> 8));
		buffer.put((byte) ((Float.floatToIntBits(data) & 0x00ff0000) >>> 16));
		buffer.put((byte) ((Float.floatToIntBits(data) & 0xff000000) >>> 24));
	}

	public static void setDouble(ByteBuffer buffer, double data) {

		assert (buffer.capacity() - buffer.position() >= 8);

		buffer.put((byte) (Double.doubleToLongBits(data) & 0x00000000000000ffl));
		buffer.put((byte) ((Double.doubleToLongBits(data) & 0x000000000000ff00l) >>> 8));
		buffer.put((byte) ((Double.doubleToLongBits(data) & 0x0000000000ff0000l) >>> 16));
		buffer.put((byte) ((Double.doubleToLongBits(data) & 0x00000000ff000000l) >>> 24));
		buffer.put((byte) ((Double.doubleToLongBits(data) & 0x000000ff00000000l) >>> 32));
		buffer.put((byte) ((Double.doubleToLongBits(data) & 0x0000ff0000000000l) >>> 40));
		buffer.put((byte) ((Double.doubleToLongBits(data) & 0x00ff000000000000l) >>> 48));
		buffer.put((byte) ((Double.doubleToLongBits(data) & 0xff00000000000000l) >>> 56));
	}

	public static String getString(ByteBuffer buffer, int size) {

		assert (0 < size);
		assert (buffer.capacity() - buffer.position() >= size);

		ByteBuffer buf = ByteBuffer.allocate(size);

		buffer.get(buf.array(), 0, size);

		try {
			return Charset.defaultCharset().newDecoder().decode(buf).toString()
					.trim();
		} catch (Exception e) {
			e.printStackTrace();
			assert (false);

			return "";
		}
	}

	public static void setString(ByteBuffer buffer, String data) {

		assert (null != data);

		byte[] bytes = data.getBytes();
		assert (buffer.capacity() - buffer.position() >= Array.getLength(bytes));

		buffer.put(bytes);
		buffer.put((byte) 0);
	}
}