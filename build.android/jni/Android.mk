#
# jxfengzi@gmail.com
#
# 2011-6-24
#
#
#

LOCAL_PATH := $(call my-dir)

#--------------------------------------------------------------------
# test
#--------------------------------------------------------------------
include $(CLEAR_VARS)

SRC_DIR          := ../../src
LOCAL_MODULE     := test
LOCAL_SRC_FILES  := $(SRC_DIR)/test.c
LOCAL_C_INCLUDES := ../mdnsresponder/mDNSShared
LOCAL_CFLAGS     := -fPIC           \
                    -D__GLIBC__     \
                    -D__ANDROID__
#LOCAL_LDLIBS     := -Llib64/ -lmdnssd -lcutils
LOCAL_LDLIBS     := -Llib64/ -lmdnssd 

include $(BUILD_EXECUTABLE)
