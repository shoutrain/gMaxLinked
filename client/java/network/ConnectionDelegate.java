package gMaxLinked.exmpale.android.network;

import gMaxLinked.exmpale.android.msg.Msg;

public interface ConnectionDelegate {

	public void onConnected();

	public void onDisconnected(boolean byRemote);

	public void onMessage(Msg msg);

	public void onError();
}
