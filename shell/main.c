#include <assert.h>
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
        case '\t':
            break;
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

static int do_exec(char *path, int argc, char *argv[])
{
    int pid, status;
    struct stat buf;

    if (stat(path, &buf) != 0 || (buf.st_mode & S_IXUSR) == 0) { 
        printf("exec: not an executable file\n");
        return 1;
    }
    if (buf.st_mode & S_IFDIR) {
        return chdir(path);
    }

    if ((pid = fork()) == 0) {
        execve(path, argv, NULL);
    }
    if (pid < 0) {
        printf("fork failed\n");
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

static char *envpath[16];

static int setup_envpath(char *envp[])
{
    int i;
    char *path;

    memset(envpath, 0x00, sizeof(char *) * 16);
    for (i = 0, path = NULL; envp[i]; i++) {
        if (!strncmp("PATH=", envp[i], 5)) {
            path = envp[i];
            break;
        }
    }
    if (!path)
        return -1;
    path = path + 5;
    i = 0;
    while (*path != NULL) {
        envpath[i++] = path;
        while (*path && *path != ':') path++;
        if (*path)
            *path++ = 0;
    }
    return 0;
}

static char pathbuf[512];

static char *find_command(char *cmd)
{
    int i;
    struct stat sbuf;

    if (!strncmp(".", cmd, 1) || !strncmp("/", cmd, 1)) {
        strcpy(pathbuf, cmd);
        return pathbuf;
    }
    for (i = 0; envpath[i]; i++) {
        strcpy(pathbuf, envpath[i]);
        strcat(pathbuf, "/");
        strcat(pathbuf, cmd);
        if (stat(pathbuf, &sbuf) == 0 && sbuf.st_mode & S_IXUSR) {
            return pathbuf;
        }
    }
    return NULL;
}

static void echo_prompt()
{
    printf("# ");
}

int main(int argc, char *argv[], char *envp[])
{
    int len, cargc;
    int (*func)(int, char **);
    char *cargv[32], *path;

    if (setup_envpath(envp) < 0) {
        panic("sh: setup_envpath failed\n");
        return -1;
    }

    // cls(0, NULL);
    while (1) {
        echo_prompt();
        len = getline(buffer);
        memset(cargv, 0x00, sizeof(char *) * 32);
        if (!(cargc = parse_cmd(buffer, len, cargv))) {
            continue;
        }

        if ((func = match_builtin(cargv[0]))) {
            cmdretval = func(cargc, cargv);
            continue;
        }

        if ((path = find_command(cargv[0]))) {
            cmdretval = do_exec(path, cargc, cargv);
            continue;
        }
        printf("command not found\n");
    }
    return 0;
}
