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
  UNUSED(env);UNUSED(clazz);UNUSED(str);

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
  jint *ram_store = env->GetIntArrayElements(arr, 0);
  storeRom(ram_store);
  env->ReleaseIntArrayElements(arr, ram_store, 0);
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


