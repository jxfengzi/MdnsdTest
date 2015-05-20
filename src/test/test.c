#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CtBonjour.h"

int main()
{
    CtBonjour *bonjour = NULL;

    do
    {
        bonjour = CtBonjour_New();
        if (bonjour == null)
        {
            break;
        }

        CtBonjour_DiscoverService(bonjour, "_airplay._tcp", "local.");

        cmd_run();

    } while (0);

    CtBonjour_Delete(bonjour);

    return 0;
}
