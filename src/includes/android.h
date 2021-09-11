#ifndef SRC_INCLUDES_ANDROID_INCL_H_
#define SRC_INCLUDES_ANDROID_INCL_H_

#include <android/log.h>

#define APP_NAME "SdlVulkanApp"

#define LOG_INFO(...) __android_log_print(ANDROID_LOG_INFO, APP_NAME,  __VA_ARGS__)
#define LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, APP_NAME,  __VA_ARGS__)

#endif
