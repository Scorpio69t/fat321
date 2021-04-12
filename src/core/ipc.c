#include <boot/cpu.h>
#include <boot/irq.h>
#include <kernel/bugs.h>
#include <kernel/ipc.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>

long do_send(frame_t *regs, pid_t to, message *msg)
{
    proc_t *dest = map_proc(to);

    // printk("%x send to %x\n", msg->src, to);

    if (msg->src == IPC_INTR) {
        dest->has_intr = 1;
        if (dest->state == PROC_RECEIVING && (dest->wait == IPC_INTR || dest->wait == IPC_ALL))
            dest->state = PROC_RUNNABLE;
        return 0;
    }

    memcpy(&current->msg, msg, sizeof(message));
    current->wait = to;
    current->state = PROC_SENDING;
    list_add_tail(&current->wait_list, &dest->wait_proc);

    if (dest->state == PROC_RECEIVING && (dest->wait == current->pid || dest->wait == IPC_ALL)) {
        dest->state = PROC_RUNNABLE;
    }
    schedule();
    return 0;
}

long do_recv(frame_t *regs, pid_t from, message *msg)
{
    proc_t *source;

    // printk("%x recv from %x\n", msg->src, from);

    /* receive all type message */
    if (from == IPC_ALL) {
    recv_all:
        if (current->has_intr) {
            msg->type = MSG_INTR;
            msg->m_intr.type = INTR_OK;
            current->has_intr = 0;
            return 0;
        }

        if (!list_is_null(&current->wait_proc)) {
            source = list_first_entry(&current->wait_proc, proc_t, wait_list);
            list_del(&source->wait_list);
            memcpy(msg, &source->msg, sizeof(message));
            current->wait = IPC_NOWAIT;
            source->state = PROC_RUNNABLE;
            return 0;
        }

        current->wait = IPC_ALL;
        current->state = PROC_RECEIVING;
        schedule();
        goto recv_all;
    }

    /* receive intrrupter type message */
    if (from == IPC_INTR) {
        if (!current->has_intr) {
            current->wait = IPC_INTR;
            current->state = PROC_RECEIVING;
            schedule();
        }

        current->has_intr = 0;
        msg->type = MSG_INTR;
        msg->m_intr.type = INTR_OK;
        return 0;
    }

    /* receive other type message */
    source = map_proc(from);
    assert(source != NULL);

    if (!(source->state == PROC_SENDING && source->wait == current->pid)) {
        current->wait = from;
        current->state = PROC_RECEIVING;
        schedule();
    }
    list_del(&source->wait_list);
    memcpy(msg, &source->msg, sizeof(message));
    current->wait = IPC_NOWAIT;
    source->state = PROC_RUNNABLE;

    return 0;
}

long do_sendrecv(frame_t *regs, pid_t to, message *msg)
{
    int status;

    if ((status = do_send(regs, to, msg)) != 0)
        return status;
    if ((status = do_recv(regs, to, msg)) != 0)
        return status;
    return 0;
}

long process_kernel_message(message *msg)
{
    long retval;

    switch (msg->type) {
    case MSG_IRQ:
        if (msg->m_irq.type == IRQ_REGISTER)
            retval = register_irq(msg->m_irq.irq_no, current->pid);
        else if (msg->m_irq.type == IRQ_UNREGISTER)
            retval = unregister_irq(msg->m_irq.irq_no);
        else
            return -1;
        break;
    default:
        break;
    }

    msg->retval = retval;
    return 0;
}
