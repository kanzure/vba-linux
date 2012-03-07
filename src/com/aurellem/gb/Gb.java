package com.aurellem.gb;

import java.nio.ByteBuffer;


public class Gb {
 
    
    public Gb(){}


    /** 
     * Hello World! This is just to test the native interface.
     */
    public native void sayHello();
    
    /** 
     * Run the emulator on a given rom
     * @param rom - the name of the rom.
     */
    public native void startEmulator(String rom);

}
