#include <boot/irq.h>
#include <boot/memory.h>
#include <boot/process.h>
#include <boot/system.h>
#include <kernel/bugs.h>
#include <kernel/elf.h>
#include <kernel/exec.h>
#include <kernel/gfp.h>
#include <kernel/ipc.h>
#include <kernel/kernel.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/slab.h>

static int count(char *const arg[])
{
    int nr = 0;
    while (arg && arg[nr] != 0) {
        nr++;
    }
    return nr;
}

static int open_exec(const char *pathname)
{
    message mess;

    /* TODO: 检查是否为可执行文件，实现ext2后 */
    mess.type = MSG_OPEN;
    mess.src = current->pid;
    mess.m_open.filepath = (char *)kmap((void *)pathname);
    mess.m_open.oflag = 0; /* Read Only */
    if (do_sendrecv(NULL, IPC_VFS, &mess) != 0)
        return -1;
    return mess.retval;
}

static ssize_t read_exec(int fd, void *buf, size_t nbytes)
{
    message mess;

    mess.type = MSG_READ;
    mess.src = current->pid;
    mess.m_read.fd = fd;
    mess.m_read.buf = kmap(buf);
    mess.m_read.size = nbytes;
    if (do_sendrecv(NULL, IPC_VFS, &mess) != 0)
        return -1;
    return mess.retval;
}

static off_t lseek_exec(int fd, off_t offset, int whence)
{
    message mess;

    mess.type = MSG_LSEEK;
    mess.src = current->pid;
    mess.m_lseek.fd = fd;
    mess.m_lseek.offset = offset;
    mess.m_lseek.whence = whence;
    if (do_sendrecv(NULL, IPC_VFS, &mess) != 0)
        return -1;
    return mess.retval;
}

static int reallocfs_exec(void)
{
    message mess;

    mess.type = MSG_EXECFS;
    mess.src = current->pid;
    if (do_sendrecv(NULL, IPC_VFS, &mess) != 0)
        return -1;
    return 0;
}

static int check_ehdr(Elf64_Ehdr *ehdr)
{
    /* check header magic */
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 || ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3)
        goto check_failed;
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64)
        goto check_failed;
    if (ehdr->e_type != ET_EXEC)
        goto check_failed;
    if (ehdr->e_machine != EM_X86_64)
        goto check_failed;
    return 1;
check_failed:
    return 0;
}

/**
 * 将用户空间中的参数暂存到内核空间
 * Return: - 成功 返回参数暂存的参数个数, 并修改prt_argv为新的地址
 *         - 失败 返回-1
 */
static int save_argv(const char *pathname, char **ptr_argv[], char *buf, int bufsz)
{
    char **newargv, **argv;
    int    argc, len, i;

    argv = *ptr_argv;
    argc = count(argv);
    newargv = (char **)buf;
    if (bufsz - (argc + 1) * sizeof(char *) < 0)
        return -1;
    buf += (argc + 1) * sizeof(char *);  // argv中不包含pathname参数
    bufsz -= (argc + 1) * sizeof(char *);

    len = strlen(pathname) + 1;  // 包括\0
    if (bufsz - len < 0)
        return -1;
    memcpy(buf, (void *)pathname, len);
    newargv[0] = buf;
    buf += len;
    bufsz -= len;

    for (i = 0; i < argc; i++) {
        len = strlen(argv[i]) + 1;
        if (bufsz - len < 0)
            return -1;
        memcpy(buf, argv[i], len);
        newargv[i + 1] = buf;
        buf += len;
        bufsz -= len;
    }
    *ptr_argv = newargv;
    return argc + 1;
}

/**
 * copy_argv - 向栈拷贝参数, 并设置argv为新地址
 * @argc: 参数数量
 * @argv: 需要拷贝的参数
 * @bufend: 缓冲区结束位置，也是栈底, 需要8字节对齐
 * @bufsize: 缓冲区大小
 * Return:  - 成功 新的缓存区结束位置，新的栈底，8字节对齐
 *          - 失败 返回NULL
 */
static void *copy_argv(int argc, char **ptr_argv[], void *bufend, int bufsize)
{
    int    i, len;
    char **newargv;
    char **argv;

    argv = *ptr_argv;
    if (bufsize - argc * sizeof(char *) < 0)
        return NULL;
    bufend -= argc * sizeof(char *);
    newargv = (char **)bufend;

    for (i = 0; i < argc; i++) {
        len = strlen(argv[i]);
        if (bufsize - len - 1 < 0)
            return NULL;
        bufend -= len + 1;
        memcpy(bufend, argv[i], len + 1); /* 末尾的\0 */
        newargv[i] = (char *)bufend;
        bufsize -= len + 1;
    }

    *ptr_argv = newargv;
    return (void *)(bufend - ((unsigned long)bufend & 0x7));
}

int do_execve(const char *pathname, char *const argv[], char *const envp[])
{
    int           fd, n, i;
    int           argc;
    unsigned long end_stack, entry;
    Elf64_Ehdr *  ehdr;
    Elf64_Phdr *  phdr_table;
    char *        argvbuf;
    proc_t *      proc;

    proc = current;
    ehdr = NULL;
    argvbuf = NULL;

    if ((fd = open_exec(pathname)) < 0)
        goto failed;

    ehdr = (Elf64_Ehdr *)kmalloc(sizeof(Elf64_Ehdr), 0);
    if ((n = read_exec(fd, ehdr, sizeof(Elf64_Ehdr))) != sizeof(Elf64_Ehdr))
        goto failed;
    if (!check_ehdr(ehdr))
        goto failed;

    entry = ehdr->e_entry;
    argvbuf = (char *)get_zeroed_page(GFP_KERNEL);
    assert(argvbuf != NULL);

    if ((argc = save_argv(pathname, (char ***)&argv, argvbuf, PAGE_SIZE)) < 0)
        goto failed;

    free_proc_mm(proc);
    flash_tlb();

    /* alloc user stack */
    proc->mm.end_stack = USER_STACK_END;
    proc->mm.start_stack = USER_STACK_END;
    for (i = 0; i < (1 << USER_STACK_ORDER); i++) {
        proc->mm.start_stack -= PAGE_SIZE;
        map_page(proc, proc->mm.start_stack);
    }
    end_stack = (unsigned long)copy_argv(argc, (char ***)&argv, (void *)proc->mm.end_stack, PAGE_SIZE);
    end_stack -= 2 * sizeof(unsigned long);
    ((unsigned long *)end_stack)[0] = (unsigned long)argc;
    ((unsigned long *)end_stack)[1] = (unsigned long)argv;

    proc->mm.nr_seg = 0;
    proc->mm.start_brk = USER_BRK_START;
    proc->mm.brk = USER_BRK_START;

    phdr_table = (Elf64_Phdr *)kmalloc(ehdr->e_phnum * sizeof(Elf64_Phdr), 0);

    assert(lseek_exec(fd, ehdr->e_phoff, 0) == ehdr->e_phoff); /* SEEK_SET */
    assert(read_exec(fd, phdr_table, ehdr->e_phnum * sizeof(Elf64_Phdr)) == ehdr->e_phnum * sizeof(Elf64_Phdr));

    for (i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *phdr = &phdr_table[i];
        if (phdr->p_type != PT_LOAD)
            continue;
        proc->mm.psegs[proc->mm.nr_seg].vstart = phdr->p_vaddr;
        proc->mm.psegs[proc->mm.nr_seg].vend = phdr->p_vaddr + phdr->p_memsz;
        proc->mm.psegs[proc->mm.nr_seg].flags = phdr->p_flags;
        proc->mm.nr_seg++;

        uint64 seg_start = PAGE_LOWER_ALIGN(phdr->p_vaddr);
        uint64 seg_end = PAGE_UPPER_ALIGN(phdr->p_vaddr + phdr->p_memsz);
        int64  page_num = (seg_end - seg_start) / PAGE_SIZE;

        assert(lseek_exec(fd, phdr->p_offset, 0) == phdr->p_offset);
        uint64 filesz = phdr->p_filesz, copyaddr = phdr->p_vaddr, copysz;
        while (page_num--) {
            map_page(proc, seg_start);
            seg_start += PAGE_SIZE;
            if (filesz > 0) {
                copysz = filesz < (seg_start - copyaddr) ? filesz : seg_start - copyaddr;
                assert(read_exec(fd, (void *)copyaddr, copysz) == copysz);
                filesz -= copysz;
                copyaddr += copysz;
            }
            if (filesz == 0) {
                copysz = seg_start - copyaddr;
                memset((void *)copyaddr, 0x00, copysz);
                copyaddr += copysz;
            }
        }
    }
    reallocfs_exec();
    kfree(ehdr);
    kfree(phdr_table);
    free_page((unsigned long)argvbuf);

    disable_interrupt();
    setup_proc_context(proc, entry, end_stack);
    exec_switch_context(proc);

failed:
    if (ehdr != NULL)
        kfree(ehdr);
    if (argvbuf != NULL)
        free_page((unsigned long)argvbuf);
    return -1;
}
