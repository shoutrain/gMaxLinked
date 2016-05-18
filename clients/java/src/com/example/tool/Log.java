package com.avcon.tool;

import java.util.Date;
import java.text.SimpleDateFormat;

public class Log {

	public static void d(String tag, String info) {
		SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		System.out.println("[" + df.format(new Date()) + "]" + tag + " : " + info);
	}

	public static void e(String tag, String info) {
		SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		System.out.println("[" + df.format(new Date()) + "]" +  "* ERROR * : " + info);
	}
}
