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
    ${CtThread_Header}
    ${CtThread_Source}
    ${CtBonjour_Header}
    ${CtBonjour_Source}
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
