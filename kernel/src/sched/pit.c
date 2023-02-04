#include <sched/pit.h>
#include <sched/scheduler.h>
#include <drv/framebuffer.h>
#include <cpu/idt.h>
#include <subsys/vt.h>

pit_packet_t packet;

uint32_t tickspersec = 0;
uint64_t ticks = 0;

// timer intrerrupt handler
extern void PITHandlerEntry();
extern void PITHandler(idt_intrerrupt_stack_t *stack)
{
    vmmSwap(vmmGetBaseTable()); // swap the base table
    ticks++;
    schedulerSchedule(stack);
    picEOI();
}

// send packet and set the divisor
void pitSet(uint32_t hz)
{
    // send packet
    outb(PIT_CMD, unsafe_cast(packet, uint8_t));

    // send divisor
    tickspersec = hz;
    uint16_t div = PIT_DIV / hz;
    outb(PIT_CH_0, div & 0xFF);          // low
    outb(PIT_CH_0, (div & 0xFF00) >> 8); // high
}

// initialize the legacy programable interval timer
void pitInit()
{
    cli(); // disable intrerrupts

    // fill the packet
    packet.channel = 0b00;        // channel 0
    packet.accessmode = 0b11;     // send both low byte and high byte
    packet.operatingmode = 0b010; // mode 2, rate generator
    packet.binarymode = 0b0;      // binary mode

    pitSet(200); // 200 hz (intrerrupt every ~5 miliseconds)

    // set idt gate
    idtSetGate((void *)PITHandlerEntry, PIC_IRQ_0, IDT_InterruptGateU, true);

    // unmask IRQ 0 on PIC
    outb(PIC_MASTER_DAT, inb(PIC_MASTER_DAT) & ~0b00000001);

    printk("pit: using mode 2 at %d hz\n", K_PIT_FREQ);
}

// get ticks since startup
uint64_t pitGetTicks()
{
    return ticks;
}

// get scale
uint32_t pitGetScale()
{
    return tickspersec;
}