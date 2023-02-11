#include <cpu/smp.h>
#include <fw/bootloader.h>

void smpBootstrap()
{
    struct limine_smp_response *smp = bootloaderGetSMP();
    printk("smp: detected %d cpus and we are %d\n", smp->cpu_count, smp->bsp_lapic_id);
}