package com.aurellem.gb;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class Gb {
 
    
    public Gb(){}


    /** 
     * Hello World! This is just to test the native interface.
     */
    public static native void sayHello();
    
    /** 
     * Run the emulator on a given rom
     * @param rom - the name of the rom.
     */
    public static native void startEmulator(String rom);

    
    public static void loadVBA(){
	System.loadLibrary("vba");
    }

    public static native void step();

    public static native void step(int keymask);

    public static native void shutdown();

    public static native void saveState(ByteBuffer buffer, int size);

    public static native void loadState(ByteBuffer buffer, int size);

    public static final int SAVE_SIZE = 9000;

    public static ByteBuffer saveState(){
	ByteBuffer buf = 
	    ByteBuffer.allocateDirect(SAVE_SIZE)
	              .order(ByteOrder.nativeOrder());
	buf.clear();
	saveState(buf, SAVE_SIZE);
	buf.flip();
	return buf;
    }

    public static void loadState(ByteBuffer saveState){
	loadState(saveState, SAVE_SIZE);
    }

}
