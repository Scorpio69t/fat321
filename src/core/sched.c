/**
 * 进程调度相关
 */

#include <boot/cpu.h>
#include <boot/irq.h>
#include <boot/memory.h>
#include <boot/process.h>
#include <boot/smp.h>
#include <boot/system.h>
#include <kernel/bugs.h>
#include <kernel/elf.h>
#include <kernel/exec.h>
#include <kernel/gfp.h>
#include <kernel/ipc.h>
#include <kernel/kernel.h>
#include <kernel/list.h>
#include <kernel/mm.h>
#include <kernel/sched.h>
#include <kernel/slab.h>
#include <kernel/stdlib.h>
#include <kernel/string.h>

union proc_union init_proc_union __attribute__((__section__(".data.init_proc"))) = {
    INIT_PROC(init_proc_union.proc),
};

struct list_head __proc_hash_map[PROC_HASH_MAP_SIZE];

struct list_head proc_head;

spinlock_t proc_head_lock;

struct proc_struct *proc_idle[NR_CPUS] = {
    &init_proc_union.proc,
};

proc_t *map_proc(pid_t pid)
{
    struct list_head *head;
    proc_t *pos;
    head = &__proc_hash_map[pid % PROC_HASH_MAP_SIZE];

    list_for_each_entry(pos, head, hash_map)
    {
        if (pos->pid == pid)
            return pos;
    }
    return NULL;
}

void hash_proc(proc_t *proc)
{
    assert(proc->pid != 0);
    list_add_tail(&proc->hash_map, &__proc_hash_map[proc->pid % PROC_HASH_MAP_SIZE]);
}

void unmap_proc(pid_t pid)
{
    proc_t *proc;
    if ((proc = map_proc(pid)) != NULL)
        list_del(&proc->hash_map);
}

/**
 * 时钟中断计数器
 */
unsigned long volatile ticks = INIT_TICKS;

pid_t volatile pid = INIT_PID;

inline void ticks_plus(void)
{
    ticks++;
}

inline void update_alarm(void)
{
    struct proc_struct *p;

    list_for_each_entry(p, &proc_head, proc)
    {
        if (p->alarm == 0) {
            continue;
        }
        --p->alarm;
        if (p->alarm == 0) {
            p->state = PROC_RUNNING;
        }
    }
}

/**
 * schedule是进程的调度器，该方法在就绪进程队列中选出一个进程进行切换
 * 当前进程的调度并不涉及优先级和运行时间的一系列复杂因素，仅仅是将时间片消耗完的进程
 * 移到队尾，然后选出进程状态为PROC_RUNNING的进程作为下一个的进程
 */
void schedule(void)
{
    struct proc_struct *prev, *next, *p;

    spin_lock(&proc_head_lock);

    prev = current;
    next = NULL;
    // FIXME: Can't call printk here
    // printk("schedule %d\n", smp_processor_id());
    prev->flags &= ~NEED_SCHEDULE;
    if (prev->state == PROC_RUNNING)
        prev->state = PROC_RUNNABLE;
    if (!(prev->flags & PF_IDLE)) {
        list_del(&prev->proc);
        list_add_tail(&prev->proc, &proc_head);
    }
    list_for_each_entry(p, &proc_head, proc)
    {
        if (p->state == PROC_RUNNABLE) {
            next = p;
            break;
        }
    }
    if (next)
        next->state = PROC_RUNNING;
    spin_unlock(&proc_head_lock);

    if (!next)
        next = proc_idle[smp_processor_id()];
    next->counter = 1;

    if (prev == next)
        return;
    switch_to(prev, next, prev);
}

/**
 * sys_sleep - 进程睡眠
 * 该进程实现了秒级睡眠和毫秒级睡眠的中断处理，对应sleep和msleep两个系统调用的用户态接口，根
 * 据第一个参数的类型，来确定使用哪种类型的睡眠, 时间精度位10ms
 */
long sys_sleep(unsigned long type, unsigned long t)
{
    if (type == 0) {
        current->alarm = t * HZ;
    } else if (type == 1) {
        current->alarm = t / (1000 / HZ);
    } else {
        printk("sys_sleep error\n");
        return -1;
    }
    current->state = PROC_SENDING;
    schedule();
    return 0;
}

int sys_pause(void)
{
    current->state = PROC_SENDING;
    schedule();
    return 0;
}

static proc_t *module_proc(uint64 module_start, char *const argv[], char *const envp[])
{
    proc_t *proc;
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)module_start;
    Elf64_Phdr *phdr_table;
    unsigned long end_stack;

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

    /* create process */
    proc = (proc_t *)__get_free_pages(GFP_KERNEL, KERNEL_STACK_ORDER);
    memset(proc, 0x00, PAGE_SIZE * (1 << KERNEL_STACK_ORDER));
    proc->state = PROC_SENDING;
    proc->counter = 1;
    proc->alarm = 0;
    proc->signal = 0;
    proc->parent = &init_proc_union.proc;
    list_head_init(&proc->wait_proc);
    list_head_init(&proc->children);

    proc->mm.pgd = get_zeroed_page(GFP_KERNEL);
    memcpy((void *)proc->mm.pgd, (void *)init_proc_union.proc.mm.pgd, PAGE_SIZE);
    memset((void *)proc->mm.pgd, 0x00, PAGE_SIZE / 2);

    proc->mm.end_stack = USER_STACK_END;
    proc->mm.start_stack = USER_STACK_END;
    for (int i = 0; i < (1 << USER_STACK_ORDER); i++) {
        proc->mm.start_stack -= PAGE_SIZE;
        map_page(proc, proc->mm.start_stack);
    }

    switch_pgd(proc->mm.pgd);
    end_stack = (unsigned long)setup_args(argv, envp, (void *)proc->mm.end_stack, PAGE_SIZE);
    switch_pgd(current->mm.pgd);

    proc->mm.nr_seg = 0;
    proc->mm.start_brk = USER_BRK_START;
    proc->mm.brk = USER_BRK_START;

    setup_proc_context(proc, ehdr->e_entry, end_stack);

    phdr_table = (Elf64_Phdr *)(module_start + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++) {
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
        while (page_num--) {
            map_page(proc, seg_start);
            seg_start += PAGE_SIZE;
        }
    }

    switch_pgd(proc->mm.pgd);

    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *phdr = &phdr_table[i];
        if (phdr->p_type != PT_LOAD)
            continue;
        memcpy((void *)phdr->p_vaddr, (void *)(module_start + phdr->p_offset), phdr->p_filesz);
        if (phdr->p_memsz > phdr->p_filesz)
            memset((void *)(phdr->p_vaddr + phdr->p_filesz), 0x00, phdr->p_memsz - phdr->p_filesz);
    }
    switch_pgd(current->mm.pgd);

    proc->state = PROC_RUNNABLE;
    return proc;
check_failed:
    panic("module check failed\n");
    return NULL;
}

static void init_srv_args(char ****ptr_argvs, char ****ptr_envps)
{
    int i;
    char **argv /*, **envp */;
    char ***srv_argvs, ***srv_envps;

    srv_argvs = (char ***)kmalloc(16 * sizeof(char **), 0);
    memset(srv_argvs, 0x00, 16 * sizeof(char **));
    srv_envps = (char ***)kmalloc(16 * sizeof(char **), 0);
    memset(srv_envps, 0x00, 16 * sizeof(char **));

    argv = (char **)kmalloc(4 * sizeof(char *), 0);
    assert(argv != NULL);
    memset(argv, 0x00, 4 * sizeof(char *));
    for (i = 0; i < 3; i++) {
        argv[i] = (char *)kmalloc(64, 0);
        assert(argv[i] != NULL);
    }
    sprintf(argv[0], "bootdev=%d", kinfo.bootdev);
    sprintf(argv[1], "part=%d", kinfo.bootpart);
    sprintf(argv[2], "subpart=%d", kinfo.subpart);
    srv_argvs[IPC_VFS] = argv;

    *ptr_argvs = srv_argvs;
    *ptr_envps = srv_envps;
}

static void free_srv_argv(char ***srv_argvs, char ***srv_envps)
{
    int i;
    char **argv /*, **envp */;

    argv = srv_argvs[IPC_VFS];
    for (i = 0; argv[i] != NULL; i++) kfree(argv[i]);
    kfree(argv);
    kfree(srv_argvs);
    kfree(srv_envps);
}

void proc_init(void)
{
    char ***srv_argvs, ***srv_envps;
    uint64 module_start;
    pid_t pid;

    spin_init(&proc_head_lock);
    list_head_init(&proc_head);
    for (int i = 0; i < PROC_HASH_MAP_SIZE; i++) list_head_init(&__proc_hash_map[i]);

    /* init_proc_union 中的一些属性缺失的，在这里进行补充 */
    init_proc_union.proc.mm.pgd = kinfo.global_pgd_start;

    init_srv_args(&srv_argvs, &srv_envps);
    for (int i = 0; i < kinfo.module_size; i++) {
        pid = atoi(kinfo.module[i].cmdline);
        module_start = to_vir(kinfo.module[i].mod_start);
        proc_t *proc = module_proc(module_start, srv_argvs[pid], srv_envps[pid]);
        assert(proc != NULL);
        proc->pid = pid;
        list_add_tail(&proc->proc, &proc_head);
        hash_proc(proc);
    }
    free_srv_argv(srv_argvs, srv_envps);
}
