#include <boot/cpu.h>
#include <kernel/bugs.h>
#include <kernel/ipc.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>

/**
 * try to send message
 * @regs: stack frame pointer
 * @from: source process, maybe a interrupt
 * @to: target process
 * @msg: IPC message
 * Return:
 *  - 0: send message successfully
 *  - -1: send message failed
 * Please DO NOT block or schedule in this function
 */
int64 try_send(frame_t *regs, pid_t from, pid_t to, message *msg)
{
    proc_t *proc = map_proc(to);
    assert(proc != NULL);
    if (proc == NULL)
        return -1;
    barrier();
    if (proc->state == PROC_RECEIVING && (proc->wait == from || proc->wait == IPC_BOTH)) {
        memcpy(&proc->msg, msg, sizeof(message));
        proc->state = PROC_RUNNABLE;
        return 0;
    }
    return -1;
}

/**
 * no test receive
 */
int64 nt_recv(frame_t *regs, pid_t from, message *msg)
{
    current->wait = from;
    current->state = PROC_RECEIVING;
    barrier();
    schedule();
    memcpy(msg, &current->msg, sizeof(message));
    return 0;
}

int64 do_send(frame_t *regs, pid_t to, message *msg)
{
    proc_t *proc = map_proc(to);

    if (proc == NULL)
        return -1;
    memcpy(&current->msg, msg, sizeof(message));
    printk("do_send: pid: %d, to: %d s: %d w: %d\n", current->pid, to, proc->state, proc->wait);
    if (!(proc->state == PROC_RECEIVING && (proc->wait == current->pid || proc->wait == IPC_BOTH))) {
        current->wait = to;
        current->state = PROC_SENDING;
        schedule();
    } else {
        /* 当为BOTH时可能存在并发问题 */
        memcpy(&proc->msg, msg, sizeof(message));
        current->wait = IPC_NOWAIT;
        proc->state = PROC_RUNNABLE;
    }
    return 0;
}

int64 do_recv(frame_t *regs, pid_t from, message *msg)
{
    proc_t *proc = map_proc(from);
    if (proc == NULL)
        return -1;
    printk("do_recv: pid: %d, from: %d s: %d, w: %d\n", current->pid, from, proc->state, proc->wait);
    if (!(proc->state == PROC_SENDING && proc->wait == current->pid)) {
        current->wait = from;
        current->state = PROC_RECEIVING;
        schedule();
    } else {
        memcpy(msg, &proc->msg, sizeof(message));
        current->wait = IPC_NOWAIT;
        proc->state = PROC_RUNNABLE;
    }
    return 0;
}

int64 do_sendrecv(frame_t *regs, pid_t to, message *msg)
{
    return 0;
}
