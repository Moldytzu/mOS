#include <cpu/smp.h>
#include <cpu/atomic.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <fw/bootloader.h>
#include <drv/serial.h>
#include <mm/vmm.h>
#include <sys/syscall.h>
#include <sched/scheduler.h>
#include <misc/logger.h>

bool smpJump;
bool smpReady[K_MAX_CORES];

uint8_t smpCores()
{
    return bootloaderGetSMP()->cpu_count;
}

void smpJumpUserspace()
{
    smpJump = true;
}

// this function is the entry point of each and every cpu but the bootstrap one
void cpuStart(struct limine_smp_info *cpu)
{
    cli();

    uint8_t id = smpID();

    printks("we're %d!\n", id);

    gdtInstall(id);
    idtInstall(id);
    vmmSwap(vmmGetBaseTable());

    printks("done %d\n", id);

    // we're ready
    smpReady[id] = true;

    // spinlock until we're ready to jump in userspace
    while (!smpJump)
        pause();

    syscallInit(); // enable system calls
    schedulerEnable();

    hang();
}

void smpBootstrap()
{
    struct limine_smp_response *smp = bootloaderGetSMP();

    logInfo("smp: we are core %d", smp->bsp_lapic_id);

    // load apropiate tables first
    gdtInit();

    gdtInstall(smp->bsp_lapic_id);

    idtInit(smp->bsp_lapic_id);

    vmmInit();

    if (smp->cpu_count == 1) // we are alone
    {
        logWarn("smp: no multicore setup detected");
        return;
    }

    smpJump = false;

    logInfo("smp: ready to start the other cores");
    for (size_t i = 0; i < smp->cpu_count; i++)
    {
        struct limine_smp_info *cpu = smp->cpus[i];

        if (cpu->lapic_id == smp->bsp_lapic_id) // don't unpark the bootstrap processor (we are it.)
            continue;

        atomicWrite((void *)&cpu->goto_address, (uint64_t)cpuStart);
    }

    // wait for the cpus to be ready
    for (int i = 1; i < smp->cpu_count; i++)
    {
        while (!smpReady[i])
            pause();
    }

    logInfo("smp: started %d cores", smp->cpu_count - 1);
}