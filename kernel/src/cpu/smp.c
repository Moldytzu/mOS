#include <cpu/smp.h>
#include <cpu/atomic.h>
#include <cpu/gdt.h>
#include <fw/bootloader.h>
#include <drv/serial.h>

// this function is the entry point of each and every cpu but the bootstrap one
void cpuStart(struct limine_smp_info *cpu)
{
    cli();
    serialWrite("hey!\n");
    gdtInstall(cpu->lapic_id);
    while (1)
        ;
}

void smpBootstrap()
{
    struct limine_smp_response *smp = bootloaderGetSMP();
    printk("smp: we are core %d\n", smp->bsp_lapic_id);

    // load apropiate tables
    gdtInstall(smp->bsp_lapic_id);

    if (smp->cpu_count == 1) // we are alone
        return;

    for (size_t i = 0; i < smp->cpu_count; i++)
    {
        struct limine_smp_info *cpu = smp->cpus[i];

        if (cpu->lapic_id == smp->bsp_lapic_id) // don't unpark the bootstrap processor (we are it.)
            continue;

        atomicWrite((void *)&cpu->goto_address, (uint64_t)cpuStart);
    }

    printk("smp: started %d cores\n", smp->cpu_count - 1);
}