#include <cpu/smp.h>
#include <cpu/atomic.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/xapic.h>
#include <cpu/fpu.h>
#include <fw/bootloader.h>
#include <drv/serial.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <sys/syscall.h>
#include <sched/scheduler.h>
#include <sched/scheduler.h>
#include <misc/logger.h>

bool smpJump;
bool smpReady[K_MAX_CORES];

uint8_t smpCores()
{
#ifdef K_SMP
    return bootloaderGetSMP()->cpu_count;
#else
    return 1;
#endif
}

void smpJumpUserspace()
{
    smpJump = true;
}

// this function is the entry point of each and every cpu but the bootstrap one
void cpuStart(struct limine_smp_info *cpu)
{
    cli();
    fpuInit();
    gdtInstall(smpID());
    idtInstall(smpID());
    vmmSwap(vmmGetBaseTable());

    // we're ready
    smpReady[smpID()] = true;

    // spinlock until we're ready to jump in userspace
    while (!smpJump)
        pause();

    tlbFlushAll();

    xapicInit(false);
    syscallInit(); // enable system calls

    tlbFlushAll();

    schedEnable();

    hang();
}

void smpBootstrap()
{
    struct limine_smp_response *smp = bootloaderGetSMP();

    logInfo("smp: we are core %d", smp->bsp_lapic_id);

    // load system tables first
    gdtInit();
    gdtInstall(smp->bsp_lapic_id);
    idtInit(smp->bsp_lapic_id);
    vmmInit();

#ifdef K_SMP
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
#endif
}