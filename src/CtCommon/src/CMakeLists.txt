#----------------------------------------------------------------------------
#
# module: sc_common
#
#----------------------------------------------------------------------------

SET(SRC        sc_common.h
               sc_api.h
               sc_typedef.h
               sc_define.h
               sc_ret.h
               sc_time.h
               sc_log.h
               sc_log_print.h
               sc_ret.c
               sc_time.c
               sc_log_print.c)

ADD_LIBRARY(sc_common_static STATIC ${SRC})