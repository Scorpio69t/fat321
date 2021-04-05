
#include <boot/memory.h>
#include <boot/sched.h>
#include <kernel/bugs.h>
#include <kernel/gfp.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <kernel/slab.h>
#include <kernel/string.h>

void *kmap(void *uaddr)
{
    uint64  paddr;
    uint64 *pml4, *pdpt, *pd, *pt;
    uint64  pml4_ind, pdpt_ind, pd_ind, pt_ind;
    uint64  addr = (uint64)uaddr;
    if ((addr & ((uint64)0xffff << 48)) != 0)
        return uaddr;

    pml4 = (uint64 *)current->mm.pgd;
    pml4_ind = addr / ((uint64)1 << 39);
    pdpt = (uint64 *)to_vir(pml4[pml4_ind] & -((uint64)1 << 12));
    addr = addr & (((uint64)1 << 39) - 1);
    pdpt_ind = addr / ((uint64)1 << 30);
    pd = (uint64 *)to_vir(pdpt[pdpt_ind] & -((uint64)1 << 12));
    addr = addr & (((uint64)1 << 30) - 1);
    pd_ind = addr / ((uint64)1 << 21);
    pt = (uint64 *)to_vir(pd[pd_ind] & -((uint64)1 << 12));
    addr = addr & (((uint64)1 << 21) - 1);
    pt_ind = addr / ((uint64)1 << 12);
    paddr = (pt[pt_ind] & -((uint64)1 << 12)) | (addr % ((uint64)1 << 12));
    return (void *)to_vir(paddr);
}

uint64 map_page(proc_t *proc, uint64 ustart)
{
    assert(proc->mm.pgd != NULL);
    uint64 *pml4, *pdpt, *pd, *pt, *page;
    uint64  pml4_ind, pdpt_ind, pd_ind, pt_ind;

    pml4 = pdpt = pd = pt = NULL;
    pml4 = (uint64 *)proc->mm.pgd;
    ustart = ustart & (((uint64)1 << 48) - 1);
    pml4_ind = ustart / ((uint64)1 << 39);

    if (pml4[pml4_ind] == 0) {
        pdpt = (uint64 *)get_zeroed_page(GFP_KERNEL);
        assert(pdpt != NULL);
        pml4[pml4_ind] = to_phy(pdpt) | PML4E_ATTR;
    } else {
        pdpt = (uint64 *)(to_vir(pml4[pml4_ind] & -((uint64)1 << 12)));
    }

    ustart = ustart & (((uint64)1 << 39) - 1);
    pdpt_ind = ustart / ((uint64)1 << 30);
    if (pdpt[pdpt_ind] == 0) {
        pd = (uint64 *)get_zeroed_page(GFP_KERNEL);
        assert(pd != NULL);
        pdpt[pdpt_ind] = to_phy(pd) | PDPTE_ATTR;
    } else {
        pd = (uint64 *)(to_vir(pdpt[pdpt_ind] & -((uint64)1 << 12)));
    }

    ustart = ustart & (((uint64)1 << 30) - 1);
    pd_ind = ustart / ((uint64)1 << 21);
    if (pd[pd_ind] == 0) {
        pt = (uint64 *)get_zeroed_page(GFP_KERNEL);
        assert(pt != NULL);
        pd[pd_ind] = to_phy(pt) | PDE_ATTR;
    } else {
        pt = (uint64 *)(to_vir(pd[pd_ind] & -((uint64)1 << 12)));
    }

    ustart = ustart & (((uint64)1 << 21) - 1);
    pt_ind = ustart / ((uint64)1 << 12);
    if (pt[pt_ind] == 0) {
        page = (uint64 *)get_zeroed_page(GFP_KERNEL);
        assert(page != NULL);
        pt[pt_ind] = to_phy(page) | PTE_ATTR;
    } else {
        page = (uint64 *)(to_vir(pt[pt_ind]) & -((uint64)1 << 12));
    }

    return (uint64)page;
}

uint64 unmap_page(proc_t *proc, uint64 ustart)
{
    return 0;
}

static void fill_pml4(uint64 base, uint64 addr, uint32 nr)
{
    uint64 *pml4e = (uint64 *)base;
    memset(pml4e, 0x00, PAGE_SIZE);
    for (int i = 0; i < nr; i++, addr += PAGE_SIZE) {
        pml4e[i + 256] = to_phy(addr) | PML4E_ATTR;
    }
}

static void fill_pdpt(uint64 base, uint64 addr, uint32 nr)
{
    uint64 *pdpte = (uint64 *)base;
    memset(pdpte, 0x00, upper_div(nr, PRE_PAGE_ENTRY) * PAGE_SIZE);
    for (int i = 0; i < nr; i++, addr += PAGE_SIZE) {
        pdpte[i] = to_phy(addr) | PDPTE_ATTR;
    }
}

static void fill_pd(uint64 base, uint64 addr, uint32 nr)
{
    uint64 *pde = (uint64 *)base;
    memset(pde, 0x00, upper_div(nr, PRE_PAGE_ENTRY) * PAGE_SIZE);
    for (int i = 0; i < nr; i++, addr += PAGE_SIZE) {
        pde[i] = to_phy(addr) | PDE_ATTR;
    }
}

static void fill_pt(uint64 base, uint64 addr, uint32 nr)
{
    uint64 *pte = (uint64 *)base;
    memset(pte, 0x00, upper_div(nr, PRE_PAGE_ENTRY) * PAGE_SIZE);
    for (int i = 0; i < nr; i++, addr += PAGE_SIZE) {
        pte[i] = to_phy(addr) | PTE_ATTR;
    }
}

uint64 setup_page_table(uint64 memsize)
{
    uint64 nr_pml4e, nr_pdpte, nr_pde, nr_pte;
    uint64 base_pml4, base_pdpt, base_pd,
        base_pt; /* the linear address of PML4 table, page directory pointer table, page directory, page table */
    nr_pte = memsize / PAGE_SIZE; /* ignore a little memory */
    nr_pde = upper_div(nr_pte, PRE_PAGE_ENTRY);
    nr_pdpte = upper_div(nr_pde, PRE_PAGE_ENTRY);
    nr_pml4e = upper_div(nr_pdpte, PRE_PAGE_ENTRY);

    base_pml4 = kinfo.global_pgd_start;
    base_pdpt = base_pml4 + upper_div(nr_pml4e, PRE_PAGE_ENTRY) * PAGE_SIZE;
    base_pd = base_pdpt + upper_div(nr_pdpte, PRE_PAGE_ENTRY) * PAGE_SIZE;
    base_pt = base_pd + upper_div(nr_pde, PRE_PAGE_ENTRY) * PAGE_SIZE;

    fill_pml4(base_pml4, base_pdpt, nr_pml4e);
    fill_pdpt(base_pdpt, base_pd, nr_pdpte);
    fill_pd(base_pd, base_pt, nr_pde);
    fill_pt(base_pt, KERNEL_OFFSET, nr_pte);

    switch_pgd(base_pml4);
    return base_pt + upper_div(nr_pte, PRE_PAGE_ENTRY) * PAGE_SIZE;
}
