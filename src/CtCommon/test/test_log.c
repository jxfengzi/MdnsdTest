#include "sc_log.h"

void test_0()
{
    LOG_OPEN(NULL);

    LOG_D("main", "hello %s", "world");
    LOG_V("main", "hi %s", "world");
    LOG_I("main", "1 %s", "world");
    LOG_E("main", "2 %s", "world");
    LOG_W("main", "3 %s", "world");
    LOG_D("main", "4 %s", "world");
    LOG_D("main", "hooo");

    LOG_CLOSE();
}

#ifdef _WIN32
#define LOG_FILE "d:/github.ouyang/log.txt"
#else
#define LOG_FILE "log.txt"
#endif

void test_1()
{
    LOG_OPEN(LOG_FILE);

    LOG_D("main", "hello %s", "world");
    LOG_V("main", "hi %s", "world");
    LOG_I("main", "1 %s", "world");
    LOG_E("main", "2 %s", "world");
    LOG_W("main", "3 %s", "world");
    LOG_D("main", "4 %s", "world");
    LOG_D("main", "hooo");

    LOG_CLOSE();
}

int main(int argc, char *argv[])
{
    test_0();
    test_1();

    return 0;
}
