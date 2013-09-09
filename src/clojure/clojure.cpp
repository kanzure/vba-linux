#include "com_aurellem_gb_Gb.h"
#include "../sdl/Drive.h"
#include "../gb/GB.h"
#include "../gb/gbMemory.h"
#include "../gb/gbCheats.h"
#include "../gb/gbGlobals.h"

// for VBAMovieOpen
#include "../common/movie.h"

#include <string.h>

#define UNUSED(x)  (void)(x)

extern int showScreen;
extern u16 currentButtons[4];

extern "C" {
    void say_hello(void) {
        printf("hello cruel world\n");
    }

    void step_until_f12(void) {
        stepUntilCapture();
    }

    void emu_step(int keymask) {
        step(keymask);
    }

    void emu_tick() {
        tick();
    }

    int get_current_buttons() {
        return currentButtons[0];
    }

    void set_showScreen(int status) {
        showScreen = status;
    }

    // dunno if this is correct or useful
    int get_rom_bank() {
        return gbDataMBC3.mapperROMBank;
    }

    void write_memory_at(u16 address, u8 value) {
        // borked? gbWriteMemory(address, value);
        gbWriteMemoryQuick(address, value);
    }

    int read_memory_at(u16 address) {
        return (int) gbReadMemoryQuick(address);
    }

    // this is probably a memory leak
    /*
    char * buggy_get_state() {
        char * buffer = new char[MAX_SAVE_SIZE];
        gbWriteMemSaveStatePos(buffer, MAX_SAVE_SIZE);
        return buffer;
    }*/

    void get_state(char * buffer, int size) {
        gbWriteMemSaveStatePos(buffer, size);
    }

    void set_state(char * buffer, int size) {
        gbReadMemSaveState(buffer, size);
    }

    // 65536 bytes (0x10000)
    void get_memory(int32 * memory) {
        storeMemory(memory);
    }

    void set_memory(int32 * new_memory) {
        writeMemory(new_memory);
    }

    // 29 bytes
    void get_registers(int32 * registers) {
        storeRegisters(registers);
    }

    void set_registers(int32 * new_registers) {
        setRegisters(new_registers);
    }

    int get_ram_size() {
        return getRamSize();
    }

    int get_rom_size() {
        return getRomSize();
    }

    // TODO: why doesn't this work with ctypes.c_int.in_dll(self._vba, "MAX_SAVE_SIZE").value ?
    const int MAX_SAVE_SIZE = 20000;

    int get_max_save_size() {
        return MAX_SAVE_SIZE;
    }

    void get_ram(int32 * ram) {
        storeRam(ram);
    }

    // size: 0x8000
    void get_wram(int32 * wram) {
        storeWRam(wram);
    }

    // size: 0x4000
    void get_vram(int32 * vram) {
        storeVRam(vram);
    }

    void get_rom(int32 * rom) {
        storeRom(rom);
    }

    void set_rom(int32 * new_rom) {
        writeRom(new_rom);
    }

    void save_png(const char *path) {
        gbWritePNGFile(path);
    }

    int get_cheat_count() {
        return gbCheatNumber;
    }

    bool cheat_read_gameshark_file(const char *path) {
        return gbCheatReadGSCodeFile(path);
    }

    void cheat_add_gameshark(const char *code, const char *description) {
        gbAddGsCheat(code, description);
    }

    void cheat_add_gamegenie(const char *code, const char *description) {
        gbAddGgCheat(code, description);
    }

    void cheat_enable(int id) {
        gbCheatEnable(id);
    }

    void cheat_disable(int id) {
        gbCheatDisable(id);
    }

    void cheat_remove(int id) {
        gbCheatRemove(id);
    }

    void cheat_remove_all(void) {
        gbCheatRemoveAll();
    }

    // TODO: Java_com_aurellem_gb_Gb_VBAMovieOpen
    // TODO: Java_com_aurellem_gb_Gb_getPixels
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    sayHello
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_sayHello
(JNIEnv *env, jclass clazz){
  UNUSED(env);UNUSED(clazz);
  printf("Hello from GB\n");  
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    startEmulator
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_startEmulator
(JNIEnv *env, jclass clazz, jstring str){
  UNUSED(clazz);

  const char *_romName = env->GetStringUTFChars(str, 0);
  size_t len = strlen(_romName);
  
  char romName[len + 1];

  strcpy(romName, _romName);
 
  char* arguments[] = {"vba-rlm", romName};
  runVBA(2, arguments);
}




/*
 * Class:     com_aurellem_gb_Gb
 * Method:    step
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_step
(JNIEnv *env, jclass clazz){
  step();
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    step
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_nstep
(JNIEnv *env, jclass clazz, jint keymask){
  step(keymask);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    stepUntilCapture
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_stepUntilCapture
(JNIEnv *env, jclass clazz){
  stepUntilCapture();
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    ntick
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_aurellem_gb_Gb_ntick
(JNIEnv *env, jclass clazz){
  return tick();

}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    shutdown
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_shutdown
(JNIEnv *env, jclass clazz){
  shutdown();

}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    saveState
 * Signature: (Ljava/nio/ByteBuffer;I)J
 */
JNIEXPORT jlong JNICALL Java_com_aurellem_gb_Gb_saveState
(JNIEnv *env, jclass clazz, jobject buffer, jint size){
  char* buffer_address = 
    ((char*) env->GetDirectBufferAddress(buffer));
  long limit = gbWriteMemSaveStatePos(buffer_address, size);
  return limit;
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    loadState
 * Signature: (Ljava/nio/ByteBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_loadState
(JNIEnv *env, jclass clazz, jobject buffer, jint size){
  char* buffer_address = 
    ((char*) env->GetDirectBufferAddress(buffer));
  gbReadMemSaveState(buffer_address, size);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getROMSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_aurellem_gb_Gb_getROMSize
(JNIEnv *env, jclass clazz){
  return getRomSize();
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getRAMSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_aurellem_gb_Gb_getRAMSize
(JNIEnv *env, jclass clazz){
  return getRamSize();
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getMemory
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_getMemory
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *mem_store = env->GetIntArrayElements(arr, 0);
  storeMemory(mem_store);
  env->ReleaseIntArrayElements(arr, mem_store, 0);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    writeMemory
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_writeMemory
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *new_memory = env->GetIntArrayElements(arr, 0);
  writeMemory(new_memory);
  env->ReleaseIntArrayElements(arr, new_memory, 0);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getRAM
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_getRAM
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *ram_store = env->GetIntArrayElements(arr, 0);
  storeRam(ram_store);
  env->ReleaseIntArrayElements(arr, ram_store, 0);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getROM
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_getROM
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *rom_store = env->GetIntArrayElements(arr, 0);
  storeRom(rom_store);
  env->ReleaseIntArrayElements(arr, rom_store, 0);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    writeROM
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_writeROM
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *new_rom = env->GetIntArrayElements(arr, 0);
  writeRom(new_rom);
  env->ReleaseIntArrayElements(arr, new_rom, 0);
}



/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getWRAM
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_getWRAM
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *ram_store = env->GetIntArrayElements(arr, 0);
  storeWRam(ram_store);
  env->ReleaseIntArrayElements(arr, ram_store, 0);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getVRAM
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_getVRAM
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *ram_store = env->GetIntArrayElements(arr, 0);
  storeVRam(ram_store);
  env->ReleaseIntArrayElements(arr, ram_store, 0);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getRegisters
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_getRegisters
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *registers_store = env->GetIntArrayElements(arr, 0);
  storeRegisters(registers_store);
  env->ReleaseIntArrayElements(arr, registers_store, 0);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    writeRegisters
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_writeRegisters
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *new_registers = env->GetIntArrayElements(arr, 0);
  setRegisters(new_registers);
  env->ReleaseIntArrayElements(arr, new_registers, 0);
}


int intensity[32] = {
  0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x5e, 0x6c, 0x7a, 0x88, 0x94,
  0xa0, 0xae, 0xb7, 0xbf, 0xc6, 0xce, 0xd3, 0xd9, 0xdf, 0xe3, 0xe7,
  0xeb, 0xef, 0xf3, 0xf6, 0xf9, 0xfb, 0xfd, 0xfe, 0xff, 0xff };

int influence[3][3] = 
  {
    {16,4,4},
    {8,16,8},
    {0,8,16}
  };

int* translateRGB(int* rgb, int* store){

  int	m[3][3];
  int	i,j;

  for (i=0;i<3;i++){
    for (j=0;j<3;j++){
      m[i][j] = (intensity[rgb[i]>>3]*influence[i][j]) >> 5;}}

  for (i=0;i<3;i++)
    {
      if (m[0][i]>m[1][i])
 	{
 	  j=m[0][i]; 
 	  m[0][i]=m[1][i]; 
 	  m[1][i]=j;
 	}

      if (m[1][i]>m[2][i])
 	{
 	  j=m[1][i]; 
 	  m[1][i]=m[2][i]; 
 	  m[2][i]=j;
 	}

      if (m[0][i]>m[1][i])
 	{
 	  j=m[0][i]; 
 	  m[0][i]=m[1][i]; 
 	  m[1][i]=j;
 	}

      store[i]=(((m[0][i]+m[1][i]*2+m[2][i]*4)*5) >> 4)+32;
    }
  return store;
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    translateRGB
 * Signature: ([I[I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_translateRGB
(JNIEnv *env, jclass clazz, jintArray rgb, jintArray store){
  jint *RGB_Arr = env->GetIntArrayElements(rgb, 0);
  jint *store_Arr = env->GetIntArrayElements(store,0);
  translateRGB(RGB_Arr, store_Arr);
  env->ReleaseIntArrayElements(rgb, RGB_Arr, 0);
  env->ReleaseIntArrayElements(store, store_Arr, 0);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getPixels
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_getPixels
(JNIEnv *env, jclass clazz, jintArray arr){
  jint *pixel_store = env->GetIntArrayElements(arr, 0);
  getPixels32(pixel_store);
  env->ReleaseIntArrayElements(arr, pixel_store, 0);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    nwritePNG
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_nwritePNG
(JNIEnv *env, jclass clazz, jstring filename){
  const char *_filename = env->GetStringUTFChars(filename, 0);
  gbWritePNGFile(_filename);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    loadCheatsFromFile
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_aurellem_gb_Gb_loadCheatsFromFile
(JNIEnv *env, jclass clazz, jstring filename){
    // bool gbCheatReadGSCodeFile(const char *fileName)
    const char *_filename = env->GetStringUTFChars(filename, 0);
    return gbCheatReadGSCodeFile(_filename);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    cheatEnable
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_cheatEnable
(JNIEnv *env, jclass clazz, jint id) {
    gbCheatEnable(id);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    cheatDisable
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_cheatDisable
(JNIEnv *env, jclass clazz, jint id) {
    gbCheatDisable(id);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    cheatRemoveAll
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_cheatRemoveAll
(JNIEnv *env, jclass clazz) {
    gbCheatRemoveAll();
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    cheatRemove
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_cheatRemove
(JNIEnv *env, jclass clazz, jint id) {
    gbCheatRemove(id);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    cheatAddGamegenie
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_cheatAddGamegenie
(JNIEnv *env, jclass clazz, jstring code, jstring description) {
    const char *_code = env->GetStringUTFChars(code, 0);
    const char *_description = env->GetStringUTFChars(description, 0);
    gbAddGgCheat(_code, _description);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    cheatAddGameshark
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_cheatAddGameshark
(JNIEnv *env, jclass clazz, jstring code, jstring description) {
    const char *_code = env->GetStringUTFChars(code, 0);
    const char *_description = env->GetStringUTFChars(description, 0);
    gbAddGsCheat(_code, _description);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    readMemory
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_aurellem_gb_Gb_readMemory
(JNIEnv *env, jclass clazz, jint address){
  return (jint) gbReadMemory((u16) address);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    setMemoryAt
 * Signature: (I;I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_setMemoryAt
(JNIEnv *env, jclass clazz, jint address, jint value){
    // kanzure: wtf ?
    gbWriteMemory(address, value);
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getFrameSound
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_getFrameSound
(JNIEnv *env, jclass clazz, jbyteArray arr){
  jbyte *sound_store = env->GetByteArrayElements(arr, 0);
  int i;

  
  u8* soundBytes = (u8*) soundFrameSound;
  for (i = 0; i < 44100*2; i++){
    sound_store[i] = (jbyte) soundBytes[i];
  }
  
  /*
  u8* soundBytes = (u8*) soundFinalWave;
  for (i = 0; i < 1470*2 ; i++){
    sound_store[i] = (jbyte) soundBytes[i];
  }
  */

  env->ReleaseByteArrayElements(arr, sound_store, 0);
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getSoundFrameWritten
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_aurellem_gb_Gb_getSoundFrameWritten
  (JNIEnv *env, jclass clazz){
  return soundFrameSoundWritten;
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    setSoundFrameWritten
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_setSoundFrameWritten
(JNIEnv *env, jclass clazz , jint newSoundFrameWritten){
  soundFrameSoundWritten = newSoundFrameWritten;
}


/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getFrameSound2
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_getFrameSound2
(JNIEnv *env, jclass clazz, jbyteArray arr){
  setbuf(stdout, NULL);

  jbyte *sound_store = env->GetByteArrayElements(arr, 0);
  int i;

  for (i = 0; i < 1470*2; i++){
    sound_store[i] = (jbyte) soundCopyBuffer[i];
  }
  
  env->ReleaseByteArrayElements(arr, sound_store, 0);

}

// kanzure: hope this works..
/*
 * Class:     com_aurellem_gb_Gb
 * Method:    setROMBank
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_setROMBank
  (JNIEnv *env, jclass clazz, jint bank){
  memset(&gbDataMBC3.mapperROMBank, bank, 1 * sizeof(int32));
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    getCurrentButtons
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_aurellem_gb_Gb_getCurrentButtons
(JNIEnv *env, jclass clazz){
  return currentButtons[0];
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    showScreen
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_showScreen
(JNIEnv *env, jclass clazz, jint status){
  showScreen = status;
}

/*
 * Class:       com_aurellem_gb_Gb
 * Method:      VBAMovieOpen
 * Signature:   (Ljava/lang/String;Z;)V
 *
 * VBAMovieOpen(const char *filename, bool8 read_only)
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_VBAMovieOpen(JNIEnv *env, jclass clazz, jstring filename, jboolean read_only) {
    const char *_filename = env->GetStringUTFChars(filename, 0);
    VBAMovieOpen(_filename, read_only);
}
