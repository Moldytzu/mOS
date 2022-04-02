#include <scheduler.h>
#include <pmm.h>
#include <gdt.h>
#include <bootloader.h>
#include <serial.h>

void *kernelStack;
struct sched_task tasks[0x1000];      // todo: replace this with a linked list
uint16_t lastTID = 0, currentTID = 0; // last task ID, current task ID
bool enabled = false;                 // enabled

void schedulerSchedule(struct idt_intrerrupt_stack *stack)
{
    if (!enabled)
        return; // don't do anything if it isn't enabled

    vmmSwap(vmmGetBaseTable()); // load the base table

    serialWrite("saving ");
    serialWrite(tasks[currentTID].name);
    serialWritec('\n');
    serialWritec('\r');

    printk("old: cs=%d ss=%d rip=%p rsp=%p krsp=%p\n", stack->cs, stack->ss, stack->rip, stack->rsp, stack->krsp);

    // save the registers
    memcpy(&tasks[currentTID].intrerruptStack, stack, sizeof(struct idt_intrerrupt_stack));

    // load the next task
    currentTID++;
    if (currentTID == lastTID)
        currentTID = 0; // reset tid if we're overrunning

    serialWrite("loading ");
    serialWrite(tasks[currentTID].name);
    serialWritec('\n');
    serialWritec('\r');

    uint64_t krsp = stack->krsp; // save the stack
    memcpy(stack, &tasks[currentTID].intrerruptStack, sizeof(struct idt_intrerrupt_stack));
    stack->krsp = krsp; // restore it

    printk("new: cs=%d ss=%d rip=%p rsp=%p krsp=%p\n", stack->cs, stack->ss, stack->rip, stack->rsp, stack->krsp);

    vmmSwap(tasks[currentTID].pageTable); // swap the page table
}

void schedulerInit()
{
    memset64(tasks, 0, 0x1000 * sizeof(struct sched_task) / sizeof(uint64_t)); // clear the tasks
    kernelStack = mmAllocatePage();                                            // allocate a page for the new kernel stack
    tssGet()->rsp[0] = (uint64_t)kernelStack + 4096;                           // set kernel stack in tss
}

void schedulerEnable()
{
    enabled = true; // enable
}

void schdulerAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize)
{
    uint16_t index = lastTID++;
    tasks[index].tid = index;                              // set the task ID
    memcpy(tasks[index].name, (char *)name, strlen(name)); // set the name

    // page table
    struct vmm_page_table *newTable = mmAllocatePage();                      // allocate a new page table
    memset64(newTable, 0, sizeof(struct vmm_page_table) / sizeof(uint64_t)); // clear the page table

    tasks[index].pageTable = newTable; // set the new page table

    void *stack = mmAllocatePages(stackSize / 4096); // allocate stack for the task

    struct stivale2_struct_tag_kernel_base_address *kaddr = bootloaderGetKernelAddr(); // get kernel address
    struct stivale2_struct_tag_framebuffer *framebuffer = bootloaderGetFramebuf();

    vmmMapPhys(newTable, false, true, false); // map physical memory

    for (size_t i = 0; i < 0x10000000; i += 4096) // map kernel as read-write
        vmmMap(newTable, (void *)kaddr->virtual_base_address + i, (void *)kaddr->physical_base_address + i, false, true);

    vmmMap(newTable, (void *)kernelStack, (void *)kernelStack, false, true); // map kernel stack as read-write

    for (size_t i = 0; i < stackSize; i += 4096) // map task stack as user, read-write
        vmmMap(newTable, (void *)VMM_HHDM + i, stack + i, true, true);

    for (size_t i = 0; i < execSize; i += 4096)
        vmmMap(newTable, (void *)execBase, (void *)execBase, true, true); // map task as user, read-write

    for (size_t i = 0; i < framebuffer->framebuffer_pitch * framebuffer->framebuffer_height; i += 4096) // map framebuffer as read-write
        vmmMap(newTable, (void *)framebuffer->framebuffer_addr + i, (void *)framebuffer->framebuffer_addr + i, false, true);

    // initial registers
    tasks[index].intrerruptStack.rip = (uint64_t)entry;                // set the entry point a.k.a the instruction pointer
    tasks[index].intrerruptStack.rflags = 0x202;                       // rflags, enable intrerrupts
    tasks[index].intrerruptStack.rsp = (uint64_t)VMM_HHDM + stackSize; // task stack
    tasks[index].intrerruptStack.cs = 8 * 3;                           // code segment for kernel is the first
    tasks[index].intrerruptStack.ss = 8 * 4;                           // data segment for kernel is the second
}
