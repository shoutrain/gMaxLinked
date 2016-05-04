package gMaxLinked.exmpale.android.network;

import java.io.InterruptedIOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

import android.util.Log;

import gMaxLinked.exmpale.android.application.Config;
import gMaxLinked.exmpale.android.msg.Header;
import gMaxLinked.exmpale.android.msg.Msg;
import gMaxLinked.exmpale.android.msg.MsgReq;
import gMaxLinked.exmpale.android.msg.MsgRes;

public class Connection implements Runnable {

	// Running formally
	private final static int CONNECTION_TIME_OUT = 10000; // ms
	private final static int WAIT_TIMES = 30;
	private final static int RW_TIME_OUT = 3000; // ms
	private final static int TIME_OUT = 500; // ms

	// Used to send message to service and change status
	private ConnectionDelegate listener = null;

	private Socket socket = null;
	// Maybe it's just domain name instead of ip
	private String server = Config.ANDROID_PUSH_URL;
	// It will keep the real ip
	private String ip = null;
	private int port = Config.ANDROID_PUSH_SERVER_PORT;
	private InetSocketAddress address = null;

	private boolean isRun = false;
	private boolean isRunning = false;

	public Connection(ConnectionDelegate listener) {
		assert (null != listener);
		this.listener = listener;
	}

	public synchronized boolean isRun() {
		return isRun;
	}

	public synchronized void start() {
		if (isRun) {
			return;
		}

		isRun = true;

		new Thread(this).start();
	}

	public synchronized void stop() {
		if (!isRun) {
			return;
		}

		isRun = false;

		while (true == isRunning) {
			try {
				Thread.sleep(TIME_OUT);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		close();
		listener.onDisconnected(false);
	}

	public void set(String server, int port) {
		assert (null != server && 0 < server.length());
		assert (1024 < port && 65535 > port);
		this.server = server;
		this.port = port;
	}

	public synchronized boolean send(MsgReq msg) {
		if (isRun && null != socket) {
			try {
				byte[] message = msg.getBytes();

				socket.getOutputStream().write(message);
				socket.getOutputStream().flush();

				return true;
			} catch (SocketTimeoutException e) {
				assert false;
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public void run() {
		isRunning = true;

		while (isRun) {
			if (false == running()) {
				int i = WAIT_TIMES;

				while (0 < i-- && isRun) {
					try {
						Thread.sleep(CONNECTION_TIME_OUT);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}

		isRunning = false;
	}

	ByteBuffer bufferHeader = ByteBuffer.allocate(Header.SIZE);
	ByteBuffer bufferBody = null;
	int total = 0;
	Header header = new Header();
	MsgRes res = null;

	private void close() {
		Log.d(Config.LOG_TAG, "Close socket");

		if (null != socket) {
			try {
				if (socket.isConnected()) {
					// looks there are bugs in sdk 2.1 when call socket.close,
					// and shutdown input and output first
					socket.shutdownInput();
					socket.shutdownOutput();
					socket.close();

					Log.d(Config.LOG_TAG, "Close socket successfully");
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				socket = null;
				res = null;
				total = 0;
				bufferBody = null;
			}
		}

		bufferHeader.position(0);
		bufferBody = null;
		total = 0;
		res = null;
	}

	private boolean running() {
		if (null == socket) {
			socket = new Socket();

			try {
				socket.setSoTimeout(RW_TIME_OUT);
			} catch (SocketException e) {
				e.printStackTrace();
			}

			try {
				ip = java.net.InetAddress.getByName(server).getHostAddress();
			} catch (UnknownHostException e) {
				e.printStackTrace();

				return false;
			}

			address = new InetSocketAddress(ip, port);
		}

		if (!isRun) {
			return false;
		}

		if (!socket.isConnected()) {
			try {
				socket.connect(address, CONNECTION_TIME_OUT);
			} catch (IllegalArgumentException e) {
				// the given SocketAddress is invalid or not supported
				// or
				// the timeout value is negative
				assert false;
			} catch (Exception e) {
				// error occurs while connecting
				e.printStackTrace();
				close();
				listener.onError();

				return false;
			}

			listener.onConnected();
		}

		int n = 0;

		// a little complex here
		for (;;) {
			if (!isRun) {
				return false;
			}

			if (null == res) {
				try {
					n = socket.getInputStream().read(bufferHeader.array(),
							total, bufferHeader.capacity() - total);
				} catch (InterruptedIOException e) {
					break;
				} catch (Exception e) {
					e.printStackTrace();
					close();
					listener.onDisconnected(true);

					break;
				}

				if (-1 == n) {
					// got the end of the stream
					Log.d(Config.LOG_TAG, "The connection has been closed");
					close();
					listener.onDisconnected(true);

					break;
				}

				total += n;

				if (Header.SIZE == total) {
					bufferHeader.position(0);
					total = 0;
					res = header.parseHeader(bufferHeader);

					if (null == res) {
						Log.e(Config.LOG_TAG,
								"The header of the buffer should not be null.");

						return false;
					}

					if (Header.SIZE == header.headerSize) {
						if (res.parseBody(bufferBody)) {
							listener.onMessage((Msg) res);
						}

						res = null;
					} else {
						assert (Header.SIZE < header.headerSize);
						bufferBody = ByteBuffer.allocate(header.headerSize
								- Header.SIZE);
					}
				}
			} else {
				assert (null != bufferBody);

				try {
					n = socket.getInputStream().read(bufferBody.array(), total,
							bufferBody.capacity() - total);
				} catch (Exception e) {
					e.printStackTrace();
					close();
					listener.onDisconnected(true);

					break;
				}

				if (-1 == n) {
					Log.d(Config.LOG_TAG, "Server shut down the connection");
					close();
					listener.onDisconnected(true);

					break;
				}

				total += n;

				if (header.headerSize - Header.SIZE == total) {
					if (res.parseBody(bufferBody)) {
						listener.onMessage((Msg) res);
					}

					res = null;
					total = 0;
					bufferBody = null;
				}
			}
		}

		return true;
	}
}