package com.aurellem.gb;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;

import org.tritonus.sampled.file.WaveAudioOutputStream;
import org.tritonus.share.sampled.file.TDataOutputStream;
import org.tritonus.share.sampled.file.TNonSeekableDataOutputStream;


public class WaveWriter {
    public  File targetFile;
    private WaveAudioOutputStream wao;
    private TDataOutputStream tos;
    private boolean initialized = false;
	
    public WaveWriter(File targetFile) 
	throws FileNotFoundException{
	tos = new TNonSeekableDataOutputStream
	    (new FileOutputStream(targetFile));
    }

    public void init(AudioFormat format){
	wao = new WaveAudioOutputStream
	    (format,AudioSystem.NOT_SPECIFIED, tos);
    }
		
    public void process(byte[] audioSamples, 
			AudioFormat format) {
	if (!initialized){
	    init(format);
	    initialized = true;
	}
	try {wao.write(audioSamples, 0, audioSamples.length);}
	catch (IOException e) {e.printStackTrace();}
    }

    public void cleanup() {
	try {wao.close();}
	catch (IOException e) {e.printStackTrace();}
    }
}