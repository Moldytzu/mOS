#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <vfs.h>

// vfs (rsi = call, rdx = arg1, r8 = retVal)
void vfs(uint64_t syscallNumber, uint64_t call, uint64_t arg1, uint64_t returnAddress, uint64_t retVal, uint64_t ignored, uint64_t r9, struct sched_task *task)
{
    if (retVal < alignD(task->intrerruptStack.rsp, 4096)) // prevent crashing
        return;

    uint64_t *retAddr = PHYSICAL(retVal);

    switch (call)
    {
    case 0: // file path exists
        if (arg1 < alignD(task->intrerruptStack.rsp, 4096))
        {
            *retAddr = false;
            break;
        }
        uint64_t fd = vfsOpen(PHYSICAL(arg1)); // open
        *retAddr = fd > 0;                     // if the fd is valid then the file exists
        vfsClose(fd);                          // close
        break;
    case 1: // directory path exists
        if (arg1 < alignD(task->intrerruptStack.rsp, 4096))
        {
            *retAddr = false;
            break;
        }
        const char *name, *tmp = PHYSICAL(arg1); // tmp is a backup for the name
        struct vfs_node *currentNode = vfsNodes();
        do
        {
            if (!currentNode->filesystem)
                goto next;

            name = tmp; // reset the pointer

            if (!strstarts(name, currentNode->filesystem->mountName)) // compare the mount name
                goto next;

            name += strlen(currentNode->filesystem->mountName); // move the pointer after the mount name

            if (strstarts((const char *)currentNode->path, name)) // compare the path
            {
                *retAddr = true; // exists
                return; // return to the application
            }

        next:
            currentNode = currentNode->next; // next node
        } while (currentNode);

        *retAddr = false; // doesn't exist
        break;
    default:
        break;
    }
}