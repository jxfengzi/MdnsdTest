ADD_EXECUTABLE(test_log test_log.c)
TARGET_LINK_LIBRARIES(test_log sc_common_static
                               ${OS_LIB})

ADD_EXECUTABLE(test_ret test_ret.c)
TARGET_LINK_LIBRARIES(test_ret sc_common_static
                               ${OS_LIB})
