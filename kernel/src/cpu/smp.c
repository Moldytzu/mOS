#include <cpu/smp.h>
#include <cpu/atomic.h>
#include <cpu/gdt.h>
#include <fw/bootloader.h>
#include <drv/serial.h>
#include <mm/vmm.h>

bool smpReady[K_MAX_CORES];

uint16_t smpGetCores()
{
    return bootloaderGetSMP()->cpu_count;
}

// this function is the entry point of each and every cpu but the bootstrap one
void cpuStart(struct limine_smp_info *cpu)
{
    cli();

    uint16_t id = smpID();

    printks("we're %d!\n", id);

    gdtInstall(id);
    vmmSwap(vmmGetBaseTable());

    printks("done %d\n", id);

    // we're ready
    smpReady[id] = true;

    hang();
}

void smpBootstrap()
{
    struct limine_smp_response *smp = bootloaderGetSMP();

    printk("smp: we are core %d\n", smp->bsp_lapic_id);

    // load apropiate tables first
    gdtInstall(smp->bsp_lapic_id);

    vmmInit();

    if (smp->cpu_count == 1) // we are alone
    {
        printk("smp: no multicore setup detected\n");
        return;
    }

    printk("smp: ready to start the other cores\n");

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

    printk("smp: started %d cores\n", smp->cpu_count - 1);
}