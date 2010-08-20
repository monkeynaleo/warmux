#ifdef ANDROID

#include <unistd.h>
#include <jni.h>
#include <android/log.h>
#include "SDL_thread.h"
#include "SDL_main.h"

/* JNI-C wrapper stuff */

#ifdef __cplusplus
#define C_LINKAGE "C"
#else
#define C_LINKAGE
#endif

static int isSdcardUsed = 0;

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

extern C_LINKAGE void
JAVA_EXPORT_NAME(DemoRenderer_nativeInit) ( JNIEnv*  env, jobject thiz )
{
  int argc = 1;
  char * argv[] = { "sdl" };

  if (isSdcardUsed) {
    chdir("/sdcard/Wormux");
  } else {
    chdir("/data/data/org.Wormux/files");
  }

  // Redirect output
  freopen("./stdout.txt", "wt", stdout);
  freopen("./stderr.txt", "wt", stderr);

  // Launch
  main(argc, argv);
};

extern C_LINKAGE void
JAVA_EXPORT_NAME(Settings_nativeIsSdcardUsed) ( JNIEnv*  env, jobject thiz, jint flag )
{
  isSdcardUsed = flag;
}

#undef JAVA_EXPORT_NAME
#undef JAVA_EXPORT_NAME1
#undef JAVA_EXPORT_NAME2
#undef C_LINKAGE

#endif
