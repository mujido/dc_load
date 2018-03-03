#include "myerrno.h"
#include "serial.h"

int __attribute__((__weak__, __section__(".libc")))
write(int handle, void* buffer, unsigned int len)
{
    switch (handle)
    {
    case 1:
    case 2:
        serial1SendBlocking(buffer, len);
        break;

    default:
        errno = EBADF;
        return -1;
    }

    return (int)len;
}