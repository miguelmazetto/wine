package com.shamanland.common.debug;

import java.io.PrintStream;

public final class AndroidLogPrintStream extends PrintStream {
    public AndroidLogPrintStream(String tag) {
        super(new AndroidLogStream(tag));
    }

    public AndroidLogPrintStream(int priority, String tag) {
        super(new AndroidLogStream(priority, tag));
    }

    @Override
    public void println() {
        write(Character.LETTER_NUMBER);
    }

    @Override
    public void println(boolean bool) {
        print(bool);
        println();
    }

    @Override
    public void println(char ch) {
        print(ch);
        println();
    }

    @Override
    public void println(char[] charArray) {
        print(charArray);
        println();
    }

    @Override
    public void println(double dnum) {
        print(dnum);
        println();
    }

    @Override
    public void println(float fnum) {
        print(fnum);
        println();
    }

    @Override
    public void println(int inum) {
        print(inum);
        println();
    }

    @Override
    public void println(long lnum) {
        print(lnum);
        println();
    }

    @Override
    public void println(Object obj) {
        print(obj);
        println();
    }

    @Override
    public synchronized void println(String str) {
        print(str);
        println();
    }
}
