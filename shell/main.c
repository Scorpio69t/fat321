#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shell.h"

static char buffer[512];
static int cmdretval;

// clang-format off
static struct cmdptr cmdtab[] = {
    { .name = "cls", .ptr = cls, },
    { .name = "pwd", .ptr = pwd, },
    { .name = "cd",  .ptr = cd, },
    { .name = NULL,  .ptr = NULL, },
};
// clang-format on

static int getline(char *buf)
{
    int len;
    char c;

    len = 0;
    do {
        if (read(0, &c, 1) != 1)
            break;
        switch (c) {
        case '\b':
            if (len > 0) {
                len--;
                write(1, &c, 1);
            }
            break;
        default:
            buf[len++] = c;
            write(1, &c, 1);
        }
    } while (c != '\n');
    buf[len] = 0;
    return len;
}

static int do_exec(int argc, char *argv[])
{
    int pid, status;
    struct stat buf;

    if (stat(argv[0], &buf) != 0 || (buf.st_mode & 0x40) == 0) {  // user execute
        printf("exec: not an executable file\n");
        return 1;
    }
    if (buf.st_mode & 0x4000) {  // dir
        return chdir(argv[0]);
    }

    if ((pid = fork()) == 0) {
        execve(argv[0], argv, NULL);
    }
    if (pid < 0) {
        printf("segment failed\n");
        return 1;
    }
    while (wait(&status) != pid) continue;
    return status;
}

static int parse_cmd(char *buf, int length, char *argv[])
{
    int i, next, argc;

    if (length == 0)
        return 0;
    next = 1;
    argc = 0;
    for (i = 0; i < length; i++) {
        if (isspace(buf[i])) {
            buf[i] = 0;
            next = 1;
            continue;
        }
        if (next) {
            argv[argc++] = &buf[i];
            next = 0;
        }
    }
    return argc;
}

static int (*match_builtin(char *cmd))(int, char **)
{
    int i;

    for (i = 0; cmdtab[i].name != 0 && cmdtab[i].ptr != 0; i++) {
        if (strcmp(cmd, cmdtab[i].name) == 0)
            return cmdtab[i].ptr;
    }
    return 0;
}

static void echo_prompt()
{
    printf("$ ");
}

int main(int argc, char *argv[], char *envp[])
{
    int len, cargc;
    int (*func)(int, char **);
    char *cargv[32];

    printf("sh argc %d\n", argc);
    for (int i = 0; argv[i] != 0; i++) printf("%s\n", argv[i]);
    printf("====\n");
    for (int i = 0; envp[i] != 0; i++) printf("%s\n", envp[i]);

    // cls(0, NULL);
    while (1) {
        echo_prompt();
        len = getline(buffer);
        memset(cargv, 0x00, sizeof(char *) * 32);
        if (!(cargc = parse_cmd(buffer, len, cargv))) {
            continue;
        }
        func = match_builtin(cargv[0]);
        if (func) {
            cmdretval = func(cargc, cargv);
            continue;
        }
        if (!strncmp(".", cargv[0], 1) || !strncmp("/", cargv[0], 1)) {
            cmdretval = do_exec(cargc, cargv);
            continue;
        }
        printf("command not found\n");
    }
    return 0;
}
