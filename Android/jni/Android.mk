LOCAL_PATH := $(call my-dir)

#
# FMOD Ex Shared Library
# 
include $(CLEAR_VARS)

LOCAL_MODULE            := fmodex
LOCAL_SRC_FILES         := ../../../fmodapi44430android/api/lib/$(TARGET_ARCH_ABI)/libfmodex.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../fmodapi44430android/api/inc

include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE    := Echoes
LOCAL_SRC_FILES := Echoes.cpp
LOCAL_LDLIBS 		   := -llog
LOCAL_SHARED_LIBRARIES := fmodex

include $(BUILD_SHARED_LIBRARY)
