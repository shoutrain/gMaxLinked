package com.exmaple.network;

import java.nio.ByteBuffer;

import com.avcon.msg.Ack;
import com.avcon.msg.Header;
import com.avcon.msg.MsgDefine;
import com.avcon.msg.MsgHandshake;
import com.avcon.msg.MsgHelper;
import com.avcon.tool.Log;

public class Tester implements ConnectionDelegate {

	private Connection cnn = null;

	@Override
	public void onConnected(Connection cnn) {
		Log.d("Tester", "onConnected");

		if (null == this.cnn) {
			this.cnn = cnn;
		}

		Header h = new Header(MsgDefine.MT_CONTROL, MsgDefine.MC_HAND_SHAKE, 0x0100, (short) 1, 0,
				System.currentTimeMillis(), 0);
		MsgHandshake b = new MsgHandshake(1, "ABCDEFGHIJKLMNOP");

		this.cnn.send(MsgHelper.compose(h, null, b));
		Log.d("Tester", "send handshake");
	}

	@Override
	public void onDisconnected(boolean byRemote) {
		Log.d("Tester", "onDisconnected");
	}

	@Override
	public boolean onMessage(ByteBuffer bufferMsg) {
		Header header = MsgHelper.parseHeader(bufferMsg);
		Ack ack = null;

		if (null == header) {
			Log.e("Tester", "invalid header");
			return false;
		}

		if (MsgDefine.MT_SIGN_ACK == (header.type & MsgDefine.MT_SIGN_ACK)) {
			ack = MsgHelper.parseAck(bufferMsg);

			if (null == ack) {
				Log.e("Tester", "invalid ack");
				return false;
			}
		}

		switch (header.cmd) {
		case MsgDefine.MC_HAND_SHAKE:
			Log.d("Tester", "get handshake");
			
			if (null == ack || 0 != ack.code) {
				Log.e("Tester", "get wrong ack for handshake, stop the connection");

				if (null != this.cnn) {
					this.cnn.stop();
				}
			} else {
				Log.d("Tester", "handshake successfully");
			}

			break;
		case MsgDefine.MC_HEART_BEAT: {
			Log.d("Tester", "get heartbeat");
			
			Header h = new Header(MsgDefine.MT_CONTROL, MsgDefine.MC_HEART_BEAT, 0x0100, (short) 1, header.seq,
					System.currentTimeMillis(), 0);
			Ack a = new Ack(0);

			if (null != this.cnn) {
				this.cnn.send(MsgHelper.compose(h, a, null));
				Log.d("Tester", "send heartbeat");
			}
		}

			break;
		default:
			Log.e("Tester", "unknown message");
			return false;
		}

		return true;
	}

	@Override
	public void onError() {
		Log.d("Tester", "onError");
	}

	public static void main(String[] args) {
		Tester tester = new Tester();
		Connection cnn = new Connection(tester);

		cnn.start();

		do {
			try {
				Thread.sleep(1000);
			} catch (Exception e) {

			}
		} while (cnn.isRun());

		cnn.stop();
	}
}
