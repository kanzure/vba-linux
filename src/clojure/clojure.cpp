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
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_step__
(JNIEnv *env, jclass clazz){
  step();
}

/*
 * Class:     com_aurellem_gb_Gb
 * Method:    step
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_step__I
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
 * Signature: (Ljava/nio/ByteBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_aurellem_gb_Gb_saveState
(JNIEnv *env, jclass clazz, jobject buffer, jint size){
  char* buffer_address = 
    ((char*) env->GetDirectBufferAddress(buffer));
  gbWriteMemSaveState(buffer_address, size);
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







