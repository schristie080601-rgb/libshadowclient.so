#include <jni.h>

extern "C" JNIEXPORT jstring JNICALL Java_modmenu_FloatingModMenuService_getTitle(JNIEnv *env, jobject /*thiz*/) {
    // Return the title used by the menu (UTF-8)
    const char* title = "下去了's Menu";
    return env->NewStringUTF(title);
}
