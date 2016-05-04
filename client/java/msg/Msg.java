package gMaxLinked.exmpale.android.msg;

public class Msg {

	// For req and ack messages
	protected Msg(int size, short id) {

		header = new Header(size + Header.SIZE, id);
	}

	// For res messages
	protected Msg(Header header) {

		this.header = new Header(header.headerSize, header.headerId);
	}

	public Header header = null;
}
