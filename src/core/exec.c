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
 * Return: - 成功 返回0
 *         - 失败 返回-1
 */
static int dup_args(char **ptr_argv[], char **ptr_envp[], char *buf, int bufsz)
{
    char **newargv, **newenvp, **argv, **envp;
    int argc, envc, len, i;

    argv = *ptr_argv;
    envp = *ptr_envp;
    argc = count(argv);
    envc = count(envp);
    if (bufsz - (argc + envc + 2) * sizeof(char *) < 0)  // argv和envp都以NULL结尾
        return -1;
    newargv = (char **)buf;
    buf += (argc + 1) * sizeof(char *);
    newenvp = (char **)buf;
    buf += (envc + 1) * sizeof(char *);
    bufsz -= (argc + envc + 2) * sizeof(char *);

    for (i = 0; i < argc; i++) {
        len = strlen(argv[i]) + 1;
        if (bufsz - len < 0)
            return -1;
        memcpy(buf, argv[i], len);
        newargv[i] = buf;
        buf += len;
        bufsz -= len;
    }
    newargv[i] = NULL;
    *ptr_argv = newargv;

    for (i = 0; i < envc; i++) {
        len = strlen(envp[i]) + 1;
        if (bufsz - len < 0)
            return -1;
        memcpy(buf, envp[i], len);
        newenvp[i] = buf;
        buf += len;
        bufsz -= len;
    }
    newenvp[i] = NULL;
    *ptr_envp = newenvp;
    return 0;
}

/**
 * setup_args - 设置用户栈上的参数
 * @ptr_argv: 指向argv的指针
 * @ptr_envp: 指向envp的指针
 * @bufend: 缓冲区结束位置，即栈底, 需要8字节对齐
 * @bufsize: 缓冲区大小
 * Return:  - 成功 新的缓存区结束位置(新的栈底)，8字节对齐
 *          - 失败 返回NULL
 */
void *setup_args(char *const argv[], char *const envp[], void *bufend, int bufsize)
{
    int i, len, argc, envc;
    char **newargv, **newenvp;
    unsigned long *arglist;

    argc = count(argv);
    envc = count(envp);
    if (bufsize - (argc + envc + 2) * sizeof(char *) < 0)
        return NULL;
    bufend -= (argc + 1) * sizeof(char *);
    newargv = (char **)bufend;
    bufend -= (envc + 1) * sizeof(char *);
    newenvp = (char **)bufend;
    bufsize -= (argc + envc + 2) * sizeof(char *);

    for (i = 0; i < argc; i++) {
        len = strlen(argv[i]) + 1;
        if (bufsize - len < 0)
            return NULL;
        bufend -= len;
        memcpy(bufend, argv[i], len);
        newargv[i] = (char *)bufend;
        bufsize -= len;
    }
    newargv[i] = NULL;

    for (i = 0; i < envc; i++) {
        len = strlen(envp[i]) + 1;
        if (bufsize - len < 0)
            return NULL;
        bufend -= len;
        memcpy(bufend, envp[i], len);
        newenvp[i] = (char *)bufend;
        bufsize -= len;
    }
    newenvp[i] = NULL;

    // align 8
    bufend = bufend - ((unsigned long)bufend & 0x7);
    bufsize = bufsize - (bufsize & 0x7);

    if (bufsize - 3 * sizeof(unsigned long *) < 0)
        return NULL;
    bufend -= 3 * sizeof(unsigned long *);
    arglist = (unsigned long *)bufend;
    arglist[0] = (unsigned long)argc;
    arglist[1] = (unsigned long)newargv;
    arglist[2] = (unsigned long)newenvp;
    return bufend;
}

int do_execve(const char *pathname, char *const argv[], char *const envp[])
{
    int fd, n, i;
    unsigned long end_stack, entry;
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr_table;
    char *argsbuf;
    proc_t *proc;

    proc = current;
    ehdr = NULL;
    argsbuf = NULL;

    if ((fd = open_exec(pathname)) < 0)
        goto failed;

    ehdr = (Elf64_Ehdr *)kmalloc(sizeof(Elf64_Ehdr), 0);
    if ((n = read_exec(fd, ehdr, sizeof(Elf64_Ehdr))) != sizeof(Elf64_Ehdr))
        goto failed;
    if (!check_ehdr(ehdr))
        goto failed;

    entry = ehdr->e_entry;
    argsbuf = (char *)get_zeroed_page(GFP_KERNEL);
    assert(argsbuf != NULL);

    if (dup_args((char ***)&argv, (char ***)&envp, argsbuf, PAGE_SIZE) != 0)
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
    end_stack = (unsigned long)setup_args(argv, envp, (void *)proc->mm.end_stack, PAGE_SIZE);

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
        int64 page_num = (seg_end - seg_start) / PAGE_SIZE;

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
    free_page((unsigned long)argsbuf);

    disable_interrupt();
    setup_proc_context(proc, entry, end_stack);
    exec_switch_context(proc);

failed:
    kfree(ehdr);
    free_page((unsigned long)argsbuf);
    return -1;
}
