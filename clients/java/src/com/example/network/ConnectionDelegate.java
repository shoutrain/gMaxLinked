package com.exmaple.network;

import java.nio.ByteBuffer;

public interface ConnectionDelegate {

	public void onConnected(Connection cnn);

	public void onDisconnected(boolean byRemote);

	public boolean onMessage(ByteBuffer bufferMsg);

	public void onError();

}
