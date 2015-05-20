#-----------------------------------------------------------------------------
#
# Bonjour
#
#-----------------------------------------------------------------------------

# CtCommon
SET(CtCommon_Header
    CtCommon/src/ct_common.h
    CtCommon/src/ct_api.h
    CtCommon/src/ct_typedef.h
    CtCommon/src/ct_define.h
    CtCommon/src/ct_ret.h
    CtCommon/src/ct_time.h
    CtCommon/src/ct_log.h
    CtCommon/src/ct_log_print.h)

SET(CtCommon_Source
    CtCommon/src/ct_ret.c
    CtCommon/src/ct_time.c
    CtCommon/src/ct_log_print.c)

SOURCE_GROUP(common\\headers        FILES     ${CtCommon_Header})
SOURCE_GROUP(common\\sources        FILES     ${CtCommon_Source})

# CtMemory
SET(CtMemory_Header
    CtMemory/src/ct_memory.h)

SET(CtMemory_Source
    CtMemory/src/ct_memory.c)

SOURCE_GROUP(memory\\headers        FILES     ${CtMemory_Header})
SOURCE_GROUP(memory\\sources        FILES     ${CtMemory_Source})

# CtSocket
SET(CtSocket_Header
    CtSocket/src/ct_socket.h
    CtSocket/src/CtSelector.h
    CtSocket/src/CtSocketIpc.h)

SET(CtSocket_Source
    CtSocket/src/ct_socket.c
    CtSocket/src/CtSelector.c
    CtSocket/src/CtSocketIpc.c)

SOURCE_GROUP(socket\\headers        FILES     ${CtSocket_Header})
SOURCE_GROUP(socket\\sources        FILES     ${CtSocket_Source})

# CtThread
SET(CtThread_Header
    CtThread/src/CtThread.h
    CtThread/src/CtMutex.h
    CtThread/src/CtCondition.h
    CtThread/src/CtSemaphore.h)

SET(CtThread_Source
    CtThread/src/CtThread.c
    CtThread/src/CtMutex.c
    CtThread/src/CtCondition.c
    CtThread/src/CtSemaphore.c)

SOURCE_GROUP(thread\\headers        FILES     ${CtThread_Header})
SOURCE_GROUP(thread\\sources        FILES     ${CtThread_Source})

# CtTimer
SET(CtTimer_Header
    CtTimer/src/CtTimer.h)

SET(CtTimer_Source
    CtTimer/src/CtTimer.c)

SOURCE_GROUP(timer\\headers         FILES     ${CtTimer_Header})
SOURCE_GROUP(timer\\sources         FILES     ${CtTimer_Source})

# CtUuid
SET(CtUuid_Header
    CtUuid/src/CtUuid.h)

SET(CtUuid_Source
    CtUuid/src/CtUuid.c)

SOURCE_GROUP(uuid\\headers          FILES     ${CtUuid_Header})
SOURCE_GROUP(uuid\\sources          FILES     ${CtUuid_Source})

# CtBonjour
SET(CtBonjour_Header
    CtBonjour/src/CtBonjour.h)

SET(CtBonjour_Source
    CtBonjour/src/CtBonjour.c)

SOURCE_GROUP(bonjour\\headers        FILES     ${CtBonjour_Header})
SOURCE_GROUP(bonjour\\sources        FILES     ${CtBonjour_Source})


SET(SRC
    ${CtCommon_Header}
    ${CtCommon_Source}
    ${CtMemory_Header}
    ${CtMemory_Source}
    ${CtSocket_Header}
    ${CtSocket_Source}
    ${CtThread_Header}
    ${CtThread_Source}
    ${CtBonjour_Header}
    ${CtBonjour_Source}
    ${CtTimer_Header}
    ${CtTimer_Source}
    ${CtUuid_Header}
    ${CtUuid_Source}
    )

#----------------------------------------------------------------------------
# static library
#----------------------------------------------------------------------------
ADD_LIBRARY(bonjour_static STATIC ${SRC})
SET_TARGET_PROPERTIES(bonjour_static PROPERTIES OUTPUT_NAME "bonjour")

#----------------------------------------------------------------------------
# test
#----------------------------------------------------------------------------
ADD_SUBDIRECTORY(test)
