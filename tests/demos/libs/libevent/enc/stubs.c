#include <openenclave/enclave.h>
#include <stdio.h>
#include <wchar.h>


int issetugid()
{
    return 0;
}

// Lib event tries opening pipes on sockets.
// We can either compile libevent without pipes
// or just fail the pipe calls so that livevent doesn't
// use them.
int pipe(int fd, int m)
{
    OE_UNUSED(fd);
    OE_UNUSED(m);
    return -1;
}

int pipe2(int fd, int m)
{
    OE_UNUSED(fd);
    OE_UNUSED(m);
    return -1;
}
