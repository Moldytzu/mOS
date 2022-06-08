#include <sys/sys.h>
#include <acpi.h>

// power (rsi = call, rbx = arg1, r8 = arg2)
void power(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, struct sched_task *task)
{
    if (call <= 1 && task->id != 1) // reboot and shutdown be available only to the init system
        return;

    switch (call)
    {
    case 0: // reboot
        acpiReboot();
        break;
    case 1: // shutdown
        hang();
        // TODO: implement this
        break;
    case 2: // get uptime
        // TODO: implement this
        break;
    default:
        break;
    }
}