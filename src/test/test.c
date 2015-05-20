#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CtBonjour.h"

int main()
{
    CtBonjour *bonjour = NULL;

    do
    {
		ct_socket_init();

        bonjour = CtBonjour_New();
        if (bonjour == NULL)
        {
            break;
        }

        CtBonjour_DiscoverService(bonjour, "_airplay._tcp", "local.", NULL);

        cmd_run();

    } while (0);

    CtBonjour_Delete(bonjour);

    return 0;
}
