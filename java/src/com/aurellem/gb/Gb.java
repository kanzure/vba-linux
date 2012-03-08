package com.aurellem.gb;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
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

    public static final int SAVE_SIZE = 90000;

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

    public static native int getROMSize();
    public static native int getRAMSize();
    

    public static final int WRAM_SIZE = 0x8000;

    public static final int VRAM_SIZE = 0x4000;

    public static final int NUM_REGISTERS = 27;

    public static native void getRAM(int[] store);

    public static native void getROM(int[] store);

    public static native void getWRAM(int[] store);

    public static native void getVRAM(int[] store);
    
    public static native void getRegisters(int[] store);
}
