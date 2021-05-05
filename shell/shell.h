#ifndef _SHELL_H_
#define _SHELL_H_

int cls(int, char **);
int pwd(int, char **);

struct cmdptr {
    char name[64];
    int (*ptr)(int, char **);
};

#endif
