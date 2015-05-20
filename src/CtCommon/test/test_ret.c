
#include <stdio.h>

#include "sc_common.h"

int main(int argc, char *argv[])
{
    char buf[128] = {0};

    printf("%s\n", sc_ret_to_str(SC_RET_OK));
    printf("%s\n", sc_ret_to_str(SC_RET_PENDING));

    printf("%s\n", sc_ret_to_str(SC_RET_E_INTERNAL));
    printf("%s\n", sc_ret_to_str(SC_RET_E_OUT_OF_MEMORY));
    printf("%s\n", sc_ret_to_str(SC_RET_E_ARG_NULL));
    printf("%s\n", sc_ret_to_str(SC_RET_E_ARG_INVALID));
    printf("%s\n", sc_ret_to_str(SC_RET_E_NOT_IMPLEMENTED));
    printf("%s\n", sc_ret_to_str(SC_RET_E_NEW));
    printf("%s\n", sc_ret_to_str(SC_RET_E_CONSTRUCT));
    printf("%s\n", sc_ret_to_str(SC_RET_E_TIMEOUT));
    printf("%s\n", sc_ret_to_str(SC_RET_E_NOT_FOUND));
    printf("%s\n", sc_ret_to_str(SC_RET_E_STARTED));
    printf("%s\n", sc_ret_to_str(SC_RET_E_STOPPED));
    printf("%s\n", sc_ret_to_str(SC_RET_E_POSITION_INVALID));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SELECT));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SOCKET_FD));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SOCKET_BIND));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SOCKET_LISTEN));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SOCKET_READ));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SOCKET_WRITE));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SOCKET_CONNECTING));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SOCKET_CONNECTED));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SOCKET_DISCONNECTED));
    printf("%s\n", sc_ret_to_str(SC_RET_E_SOCKET_ASCEPT));
    printf("%s\n", sc_ret_to_str(SC_RET_E_UUID_INVALID));
    printf("%s\n", sc_ret_to_str(SC_RET_E_HTTP_MSG_INVALID));
    printf("%s\n", sc_ret_to_str(SC_RET_E_HTTP_TYPE_INVALID));
    printf("%s\n", sc_ret_to_str(SC_RET_E_ITEM_EXIST));

    sc_getstrtime(buf, 128);
    printf("%s\n", buf);

    sc_getstrtime(buf, 128);
    printf("%s\n", buf);

    sc_getstrtime(buf, 128);
    printf("%s\n", buf);

    return 0;
}
