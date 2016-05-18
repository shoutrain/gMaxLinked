package com.exmaple.network;

import java.io.InterruptedIOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

import com.avcon.conf.Config;
import com.avcon.msg.MsgDefine;
import com.avcon.tool.Log;
import com.avcon.tool.NetworkDataHelper;

public class Connection implements Runnable {

	// Running formally
	private final static int CONNECTION_TIME_OUT = 10000; // ms
	private final static int WAIT_TIMES = 1;
	private final static int RW_TIME_OUT = 3000; // ms
	private final static int TIME_OUT = 500; // ms

	// Used to send message to service and change status
	private ConnectionDelegate listener = null;

	private Socket socket = null;
	// Maybe it's just domain name instead of ip
	private String server = Config.URL;
	// It will keep the real ip
	private String ip = null;
	private int port = Config.PORT;
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

	public synchronized boolean send(byte[] msg) {
		if (isRun && null != socket) {
			try {
				socket.getOutputStream().write(msg);
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

	private final int sizeSize = 2; // the size of size field in header
	private ByteBuffer bufferSize = ByteBuffer.allocate(sizeSize);
	
	private ByteBuffer bufferMsg = null;
	private int offset = 0;
	private int pendingSize = 0;

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
			}
		}

		bufferMsg = null;
		offset = 0;
		pendingSize = 0;
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
				assert (false);
			} catch (Exception e) {
				// error occurs while connecting
				e.printStackTrace();
				close();
				listener.onError();

				return false;
			}

			listener.onConnected(this);
		}

		int n = 0;

		// a little complex here
		for (;;) {
			if (!isRun) {
				return false;
			}

			if (0 == pendingSize) {
				try {
					n = socket.getInputStream().read(bufferSize.array(), offset, bufferSize.capacity() - offset);
				} catch (InterruptedIOException e) {
					break;
				} catch (Exception e) {
					e.printStackTrace();
					close();
					listener.onDisconnected(true);

					return false;
				}

				if (-1 == n) {
					// got the end of the stream
					Log.d(Config.LOG_TAG, "The connection has been closed");
					close();
					listener.onDisconnected(true);

					return false;
				}

				offset += n;

				if (sizeSize == offset) {
					bufferSize.position(0);
					pendingSize = NetworkDataHelper.getUShort(bufferSize);
					
					if (MsgDefine.MSG_MAX_LENGTH < pendingSize) {
						// get invalid message
						close();
						listener.onError();

						return false;
					}
					
					bufferMsg = ByteBuffer.allocate(pendingSize);
					bufferMsg.put(bufferSize);
				}
			} else {
				assert (null != bufferMsg);

				try {
					n = socket.getInputStream().read(bufferMsg.array(), offset, bufferMsg.capacity() - offset);
				} catch (Exception e) {
					e.printStackTrace();
					close();
					listener.onDisconnected(true);

					return false;
				}

				if (-1 == n) {
					Log.d(Config.LOG_TAG, "Server shut down the connection");
					close();
					listener.onDisconnected(true);

					return false;
				}

				offset+= n;

				if (pendingSize == offset) {
					if (!listener.onMessage(bufferMsg)) {
						Log.e(Config.LOG_TAG, "The message is invalid");
						close();
						listener.onError();
						
						return false;
					}

					bufferMsg = null;
					offset = 0;
					pendingSize = 0;
				}
			}
		}

		return true;
	}
}