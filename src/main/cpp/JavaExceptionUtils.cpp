#include "JavaExceptionUtils.h"
#include "JavaClassUtils.h"
#include "JavaString.h"
#include <stdarg.h>

namespace spotify {
namespace jni {

static const size_t kExceptionMaxLength = 512;

void JavaExceptionUtils::checkException(JNIEnv *env) {
  if (!env->ExceptionCheck()) {
    return;
  }

  env->ExceptionDescribe();
  std::terminate();
}

JniLocalRef<jobject> JavaExceptionUtils::newThrowable(JNIEnv *env, const char *message, ...) {
  // Find the Throwable class and its associated String constructor
  jclass throwableClazz = JavaClassUtils::findJavaClass(env, kTypeJavaClass("Throwable"));
  if (throwableClazz == NULL) {
    JavaExceptionUtils::throwRuntimeException(env, "Could not find class Throwable");
    return NULL;
  }

  const char *signature = JavaClassUtils::makeSignature(kTypeVoid, kTypeJavaString, NULL);
  jmethodID throwableCtor = env->GetMethodID(throwableClazz, "<init>", signature);
  if (throwableCtor == NULL) {
    JavaExceptionUtils::throwRuntimeException(env, "Could not find Throwable constructor");
    return NULL;
  }

  // Construct error message
  va_list arguments;
  va_start(arguments, message);
  char formattedMessage[kExceptionMaxLength];
  vsnprintf(formattedMessage, kExceptionMaxLength, message, arguments);
  va_end(arguments);
  JavaString javaMessage(formattedMessage);
  JniLocalRef<jobject> result = env->NewObject(throwableClazz, throwableCtor, javaMessage.getJavaString(env).get());
  JavaExceptionUtils::checkException(env);
  if (result == NULL) {
    JavaExceptionUtils::throwRuntimeException(env, "Could not create new Throwable instance");
    return NULL;
  }

  return result;
}


void JavaExceptionUtils::throwExceptionOfType(JNIEnv *env, const char *exception_class_name, const char *message, va_list arguments) {
  jclass clazz = JavaClassUtils::findJavaClass(env, exception_class_name);
  checkException(env);
  if (clazz == NULL) {
    env->ExceptionDescribe();
    return;
  }

  char exceptionMessage[kExceptionMaxLength];
  vsnprintf(exceptionMessage, kExceptionMaxLength, message, arguments);
  env->ThrowNew(clazz, exceptionMessage);
  env->ExceptionDescribe();
  checkException(env);
}

void JavaExceptionUtils::throwExceptionOfType(JNIEnv *env, const char *exception_class_name, const char *message, ...) {
  va_list arguments;
  va_start(arguments, message);
  throwExceptionOfType(env, exception_class_name, message, arguments);
  va_end(arguments);
}

void JavaExceptionUtils::throwException(JNIEnv *env, const char *message, ...) {
  va_list arguments;
  va_start(arguments, message);
  throwExceptionOfType(env, kTypeJavaException, message, arguments);
  va_end(arguments);
}

void JavaExceptionUtils::throwRuntimeException(JNIEnv *env, const char *message, ...) {
  va_list arguments;
  va_start(arguments, message);
  throwExceptionOfType(env, kTypeJavaClass(RuntimeException), message, arguments);
  va_end(arguments);
}

} // namespace jni
} // namespace spotify