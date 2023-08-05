#include <sys/sys.h>
#include <fw/acpi.h>

// power (rsi = call, rbx = arg1, r8 = arg2)
uint64_t power(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, sched_task_t *task)
{
    if (call <= 1 && !IS_PRIVILEGED) // reboot and shutdown be available only to the privileged apps
        return SYSCALL_STATUS_ACCESS_DENIED;

    switch (call)
    {
    case 0: // reboot
        acpiReboot();
        break;
    case 1: // shutdown
        acpiShutdown();
        break;
    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
        break;
    }
}