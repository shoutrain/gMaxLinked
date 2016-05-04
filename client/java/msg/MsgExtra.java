package gMaxLinked.exmpale.android.msg;

public class MsgExtra extends Msg {

	protected MsgExtra(int size, short id, long extra1, long extra2) {
		super(size + Extra.SIZE, id);
		extra = new Extra(extra1, extra2);
	}

	protected MsgExtra(Header header) {
		super(header);
		extra = new Extra();
	}

	public Extra extra = null;
}
