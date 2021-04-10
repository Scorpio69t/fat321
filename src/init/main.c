#include <malloc.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/syscall.h>

#include "init.h"

int main(int argc, char *argv[])
{
    int  n;
    char buf[32];
    // message m;
    // m.type = MSG_READ;
    // m.m_read.buf = buf;
    // m.m_read.size = 30;
    // while (_sendrecv(IPC_KB, &m) == 0 && m.ret > 0) {
    //     // debug("init %d\n", m.ret);
    //     buf[m.ret] = 0;
    //     debug("init %s\n", buf);
    //     m.type = MSG_READ;
    //     m.m_read.buf = buf;
    //     m.m_read.size = 30;
    // }
    while ((n = read(0, buf, 30)) > 0) {
        buf[n] = 0;
        debug("%s\n", buf);
    }
    printf("init read error %x\n", n);
    while (1) {
    }
    return 0;
}
