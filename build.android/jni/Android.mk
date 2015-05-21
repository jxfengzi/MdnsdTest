#
# jxfengzi@gmail.com
#
# 2011-6-24
#
#
#

LOCAL_PATH := $(call my-dir)

INC_CT_COMMON           := ../src/CtCommon/src
INC_CT_MEMORY           := ../src/CtMemory/src
INC_CT_SOCKET           := ../src/CtSocket/src
INC_CT_THREAD           := ../src/CtThread/src
INC_CT_TIMTER           := ../src/CtTimer/src
INC_CT_UUID             := ../src/CtUuid/src
INC_CT_BONJOUR          := ../src/CtBonjour/src
INC_MDNSD               := ../mdnsresponder/mDNSShared

INC_CETTY               := $(INC_CT_COMMON)         \
                           $(INC_CT_MEMORY)         \
                           $(INC_CT_SOCKET)         \
                           $(INC_CT_THREAD)         \
                           $(INC_CT_TIMER)          \
                           $(INC_CT_UUID)           \
                           $(INC_CT_BONJOUR)        \
                           $(INC_MDNSD)

CETTY_CFLAGS            := -D__ANDROID__            \
                           -fPIC                    \
                           -Wno-multichar

#--------------------------------------------------------------------
# bonjour_static
#--------------------------------------------------------------------
include $(CLEAR_VARS)

SRC_DIR          := ../../src
LOCAL_MODULE     := libcetty_static
LOCAL_SRC_FILES  := $(SRC_DIR)/CtCommon/src/ct_ret.c            \
                    $(SRC_DIR)/CtCommon/src/ct_time.c           \
                    $(SRC_DIR)/CtCommon/src/ct_log_print.c      \
                    $(SRC_DIR)/CtMemory/src/ct_memory.c         \
                    $(SRC_DIR)/CtSocket/src/ct_socket.c         \
                    $(SRC_DIR)/CtSocket/src/CtSelector.c        \
                    $(SRC_DIR)/CtSocket/src/CtSocketIpc.c       \
                    $(SRC_DIR)/CtThread/src/CtThread.c          \
                    $(SRC_DIR)/CtThread/src/CtMutex.c           \
                    $(SRC_DIR)/CtThread/src/CtCondition.c       \
                    $(SRC_DIR)/CtThread/src/CtSemaphore.c       \
                    $(SRC_DIR)/CtTimer/src/CtTimer.c            \
                    $(SRC_DIR)/CtUuid/src/CtUuid.c              \
                    $(SRC_DIR)/CtBonjour/src/CtBonjour.c
LOCAL_C_INCLUDES := $(INC_CETTY)
LOCAL_CFLAGS     := $(CETTY_CFLAGS)

include $(BUILD_STATIC_LIBRARY)

#--------------------------------------------------------------------
# test
#--------------------------------------------------------------------
include $(CLEAR_VARS)

SRC_DIR          := ../../src/test
LOCAL_MODULE     := test
LOCAL_SRC_FILES  := $(SRC_DIR)/test.c                           \
                    $(SRC_DIR)/command.c
LOCAL_C_INCLUDES := $(INC_CETTY)
LOCAL_CFLAGS     := -fPIC           \
                    -D__GLIBC__     \
                    -D__ANDROID__
LOCAL_STATIC_LIBRARIES := libcetty_static
#LOCAL_LDLIBS     := -Llib64/ -lmdnssd -lcutils
LOCAL_LDLIBS     := -Llib64/ -lmdnssd -llog 

include $(BUILD_EXECUTABLE)
