#include "rdt.h"

int chk_chksum(uint16_t *ptr, int len)
{
        /* _NOTE_: the length must use the host byte order */
        return (checksum(ptr, len) ? 0 : 1);
}
