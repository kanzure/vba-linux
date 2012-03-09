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

    public static native long saveState(ByteBuffer buffer, int size);

    public static native void loadState(ByteBuffer buffer, int size);

    public static final int MAX_SAVE_SIZE = 20000;

    public static ByteBuffer createDirectByteBuffer(int capacity){
	byte[] zeros = new byte[capacity];
	ByteBuffer buf = 
	    ByteBuffer.allocateDirect(capacity)
	              .order(ByteOrder.nativeOrder());
	buf.put(zeros);
	buf.clear();
	return buf;
    }

    public static ByteBuffer saveBuffer(){
	return createDirectByteBuffer(MAX_SAVE_SIZE);
    }

    public static ByteBuffer saveState(){
	ByteBuffer buf = saveBuffer();
	
	saveState(buf, buf.capacity());
	
	// determine the extent of the saved data
	int position = buf.capacity() - 1;
	for (int i = position; i > 0; i--){
	    if (0 != buf.get(i)){
		position = i;
		break;
	    }}
	System.out.println("Position: " + position);
	byte[] saveArray = new byte[position];
	ByteBuffer save = createDirectByteBuffer(position);
	buf.get(saveArray, 0 , position);
	save.put(saveArray);
	save.rewind();
	return save;
    }

    public static void loadState(ByteBuffer saveState){
	loadState(saveState, MAX_SAVE_SIZE);
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
