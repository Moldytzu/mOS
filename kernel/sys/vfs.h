#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <vfs.h>

// vfs (rsi = call, rdx = arg1, r8 = retVal)
void vfs(uint64_t syscallNumber, uint64_t call, uint64_t arg1, uint64_t returnAddress, uint64_t retVal, uint64_t ignored, uint64_t r9, struct sched_task *task)
{
    if (!INBOUNDARIES(retVal)) // prevent crashing
        return;

    struct vfs_node *currentNode;
    uint64_t *retAddr = PHYSICAL(retVal);
    const char *name, *tmp;

    switch (call)
    {
    case 0: // file path exists
        if (!INBOUNDARIES(arg1))
        {
            *retAddr = 0;
            return;
        }

        uint64_t fd = vfsOpen(PHYSICAL(arg1)); // open
        *retAddr = fd > 0;                     // if the fd is valid then the file exists
        vfsClose(fd);                          // close
        break;
    case 1: // directory path exists
        if (!INBOUNDARIES(arg1))
        {
            *retAddr = 0;
            return;
        }

        tmp = PHYSICAL(arg1); // tmp is a backup for the name
        currentNode = vfsNodes();
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
                return;          // return to the application
            }

        next:
            currentNode = currentNode->next; // next node
        } while (currentNode);

        *retAddr = false; // doesn't exist
        break;
    case 2: // list directory
        if (!INBOUNDARIES(arg1))
        {
            *retAddr = 0;
            return;
        }

        tmp = PHYSICAL(arg1); // tmp is a backup for the name
        char *retChar = (char *)retAddr;

        currentNode = vfsNodes();
        do
        {
            if (!currentNode->filesystem)
                goto next1;

            name = tmp; // reset the pointer

            if (memcmp(name, currentNode->filesystem->mountName, min(strlen(currentNode->filesystem->mountName), strlen(currentNode->path))) != 0) // compare the mount name
                goto next1;

            name += strlen(currentNode->filesystem->mountName); // move the pointer after the mount name

            if (!strstarts((const char *)currentNode->path, name)) // compare the path
                goto next1;

            if (strstarts(tmp, currentNode->filesystem->mountName) && strcmp((const char *)(tmp + strlen(currentNode->filesystem->mountName)), currentNode->path) == 0) // skip the node if the full path is equal to the path name
                goto next1;

            memcpy(retChar, currentNode->filesystem->mountName, strlen(currentNode->filesystem->mountName)); // copy the mount name
            retChar += strlen(currentNode->filesystem->mountName);                                           // move the pointer forward
            memcpy(retChar, currentNode->path, strlen(currentNode->path));                                   // copy the path
            retChar += strlen(currentNode->path);                                                            // move the pointer forward
            *(retChar++) = ' ';                                                                              // append a space
        next1:
            currentNode = currentNode->next; // next node
        } while (currentNode);

        break;
    case 3: // size of descriptor
        *retAddr = vfsSize(arg1);
        break;
    default:
        break;
    }
}