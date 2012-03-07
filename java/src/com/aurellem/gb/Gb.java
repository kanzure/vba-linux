package com.aurellem.gb;

import java.nio.ByteBuffer;


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

}
