#include <cpu/smp.h>
#include <cpu/atomic.h>
#include <fw/bootloader.h>
#include <drv/serial.h>

// this function is the entry point of each and every cpu but the bootstrap one
void cpuStart(struct limine_smp_info *cpu)
{
    serialWrite("hey!\n");
    while (1)
        ;
}

void smpBootstrap()
{
    struct limine_smp_response *smp = bootloaderGetSMP();
    printk("smp: detected %d cpus and we are %d\n", smp->cpu_count, smp->bsp_lapic_id);

    for (size_t i = 0; i < smp->cpu_count; i++)
    {
        struct limine_smp_info *cpu = smp->cpus[i];

        atomicWrite((void *)&cpu->goto_address, (uint64_t)cpuStart);
    }
}