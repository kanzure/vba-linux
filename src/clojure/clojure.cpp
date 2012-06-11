#include "com_aurellem_gb_Gb.h"
#include "../sdl/Drive.h"
#include "../gb/GB.h"

#include <string.h>

#define UNUSED(x)  (void)(x)


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

  
