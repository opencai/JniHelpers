#include "JavaThreadUtils.h"
#include "JavaExceptionUtils.h"

namespace spotify {
namespace jni {

static JavaVM* sJavaVm = NULL;

void JavaThreadUtils::initialize(JavaVM *jvm) {
  sJavaVm = jvm;
}

JNIEnv* JavaThreadUtils::getEnvForCurrentThread() {
  JNIEnv *env;

  if (sJavaVm == NULL) {
    return NULL;
  } else if (sJavaVm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
    // The current thread isn't attached to a JNIEnv, return NULL.
    return NULL;
  }

  return env;
}

JNIEnv* JavaThreadUtils::attachCurrentThreadToJVM(const char* thread_name) {
  JNIEnv *env;
  JavaVMAttachArgs args;
  int result = -1;

  args.version = JNI_VERSION_1_4;
  args.name = const_cast<char*>(thread_name);
  args.group = NULL;

#ifdef ANDROID
  result = sJavaVm->AttachCurrentThread((JNIEnv**)(&env), &args);
#else
  result = sJavaVm->AttachCurrentThread((void**)(&env), &args);
#endif
  if (result != JNI_OK) {
    JavaExceptionUtils::throwRuntimeException(env, "Could not attach thread %s to JVM", thread_name);
    return NULL;
  }

  return env;
}

void JavaThreadUtils::detatchCurrentThreadFromJVM() {
  sJavaVm->DetachCurrentThread();
}

} // namespace jni
} // namespace spotify