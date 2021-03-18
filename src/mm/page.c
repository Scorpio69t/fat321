#include <boot/bug.h>
#include <boot/memory.h>
#include <boot/sched.h>
#include <feng/gfp.h>
#include <feng/kernel.h>
#include <feng/mm.h>
#include <feng/page.h>
#include <feng/slab.h>
#include <feng/string.h>

static void fill_pml4(uint64 base, uint64 addr, uint32 nr)
{
    uint64 *pml4e = (uint64 *)base;
    memset(pml4e, 0x00, PAGE_SIZE);
    for (int i = 0; i < nr; i++, addr += PAGE_SIZE) {
        pml4e[i] = to_phy(addr) | PML4E_ATTR;
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

    base_pml4 = PAGE_TABLE_ADDRESS;
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
