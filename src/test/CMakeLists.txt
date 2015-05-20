ADD_EXECUTABLE(test test.c command.c)
TARGET_LINK_LIBRARIES(test ${DNSSD_LIB} ${OS_LIB} bonjour_static)
