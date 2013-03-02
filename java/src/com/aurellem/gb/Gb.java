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

    public static native void stepUntilCapture();

    public static native int ntick();

    public static boolean tick(){
	return (1 == ntick());
    }

    public static native void nstep(int keymask);

    public static void step(int keymask){
	if (-1 == keymask) {step();}
	else {nstep(keymask);}}

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
    
    public static final int RAM_SIZE = 0x10000;
    
    public static final int ROM_SIZE = 0x200000;

    public static final int NUM_REGISTERS = 29;

    public static final int GB_MEMORY = 0x20000;

    public static final int MAX_SOUND_BYTES = 44100*2;

    public static native void getMemory(int[] store);

    public static native void writeMemory(int[] newMemory);

    public static native void setMemoryAt(int address, int value);

    public static native void getRAM(int[] store);

    public static native void getROM(int[] store);

    public static native void writeROM(int[] newROM);

    public static native void getWRAM(int[] store);

    public static native void getVRAM(int[] store);
    
    public static native void getRegisters(int[] store);
    
    public static native void writeRegisters(int[] newRegisters);

    public static native void translateRGB(int[] rgb, int[] store);

    public static final int DISPLAY_WIDTH = 160;
     
    public static final int DISPLAY_HEIGHT = 144;

    public static native void getPixels(int[] store);

    public static native void nwritePNG(String filename);

    public static native int readMemory(int address);
    
    public static native void getFrameSound(byte[] store);

    public static native int getSoundFrameWritten();

    public static native void setSoundFrameWritten(int frames);

    public static native void getFrameSound2(byte[] store);

    public static native void setROMBank(int bank);

}
