package com.shamanland.common.debug;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;

public final class AndroidLogStream extends OutputStream {
    private final int mPriority;
    private final String mTag;
    private boolean mPaused;

    private final ByteArrayOutputStream mBuffer;

    public String getTag() {
        return mTag;
    }

    public AndroidLogStream(String tag) {
        this(Log.VERBOSE, tag);
    }

    public AndroidLogStream(int priority, String tag) {
        validate(priority, tag);
        mPriority = priority;
        mTag = tag;
        mBuffer = new ByteArrayOutputStream();
    }

    private void validate(int priority, String tag) {
        if (tag == null) throw new NullPointerException("tag is null");
        if (tag.trim().length() == 0) throw new IllegalArgumentException("empty tag");

        if (priority == Log.ASSERT) return;
        if (priority == Log.DEBUG) return;
        if (priority == Log.ERROR) return;
        if (priority == Log.INFO) return;
        if (priority == Log.VERBOSE) return;
        if (priority == Log.WARN) return;

        throw new IllegalArgumentException("uknown priority: " + priority);
    }

    @Override
    public void write(int oneByte) throws IOException {
        synchronized (mBuffer) {
            if (mPaused) {
                return;
            }

            if (Character.LETTER_NUMBER == (byte) oneByte) {
                Log.println(mPriority, mTag, mBuffer.toString());
                mBuffer.reset();
            } else {
                mBuffer.write(oneByte);
            }
        }
    }

    @Override
    public void flush() throws IOException {
        synchronized (mBuffer) {
            if (mBuffer.size() > 0) {
                write(Character.LETTER_NUMBER);
            }
        }
    }

    public void resume() throws IOException {
        synchronized (mBuffer) {
            mBuffer.reset();
            flush();

            mPaused = false;
        }
    }

    public void suspend() throws IOException {
        synchronized (mBuffer) {
            mBuffer.reset();
            flush();

            mPaused = true;
        }
    }
}
