#include <sys/sys.h>
#include <fs/vfs.h>

// count characters in str
size_t count(const char *str, char c)
{
    size_t cnt = 0;
    while (*str++)
        if (*str == c)
            cnt++;

    return cnt;
}

// vfs (rsi = call, rdx = arg1, r8 = retVal)
uint64_t vfs(uint64_t call, uint64_t arg1, uint64_t retVal, uint64_t r9, uint64_t r10, sched_task_t *task)
{
    struct vfs_node_t *currentNode;
    uint64_t *retAddr = PHYSICAL(retVal);

    switch (call)
    {
    case 0: // file path exists
    {
        if (!IS_MAPPED(arg1))
            return 0;

        return vfsExists(PHYSICAL(arg1));
    }

    case 1: // directory path exists
    {
        if (!IS_MAPPED(arg1))
            return false;

        const char *target = PHYSICAL(arg1); // target directory
        char fullPath[512];                  // full path of node

        currentNode = vfsNodes();
        do // iterate over all nodes
        {
            if (!currentNode->filesystem)
                goto next;

            vfsGetPath((uint64_t)currentNode, fullPath);

            if (strcmp(target, fullPath) == 0) // compare full paths
                return true;

        next:
            currentNode = currentNode->next; // next node
        } while (currentNode);

        return false;
    }

    case 2: // list directory
    {
        if (!IS_MAPPED(arg1) || !retAddr)
        {
            *retAddr = 0;
            return SYSCALL_STATUS_ERROR;
        }

        const char *target = PHYSICAL(arg1); // target directory to list
        char *retChar = (char *)retAddr;     // return array

        char fullPath[512]; // full path of node

        logDbg(LOG_SERIAL_ONLY, "vfs: listing directory for %s", target);

        if (strlen(target) < 1) // invalid path
            return SYSCALL_STATUS_ERROR;

        currentNode = vfsNodes();
        do
        {
            if (!currentNode->filesystem)
                goto next1;

            vfsGetPath((uint64_t)currentNode, fullPath);

            bool listFile = target[strlen(target) - 1] != '/';           // check if we list a file
            bool sameStart = strstarts(fullPath, target);                // check if path starts with the same characters
            bool sameDepth = count(fullPath, '/') == count(target, '/'); // check if the path and name has the same number of slashes
            bool isDirectory = fullPath[strlen(fullPath) - 1] == '/';    // check if the path is a directory

            if (isDirectory)
                sameDepth = count(fullPath, '/') == count(target, '/') + 1;

            // logDbg(LOG_SERIAL_ONLY, "vfs: target: %s; current: %s; %d%d%d%d", target, fullPath, listFile, sameStart, sameDepth, isDirectory);

            if (listFile && strcmp(target, fullPath) == 0) // check for identical comparison when listing a file
            {
                memcpy(retChar, currentNode->path, strlen(currentNode->path)); // copy the path
                return SYSCALL_STATUS_OK;
            }

            if (sameStart && sameDepth)
            {
                char *localPath = fullPath + strlen(target);   // get the local path
                memcpy(retChar, localPath, strlen(localPath)); // copy the local path
                retChar += strlen(localPath);                  // move the pointer forward
                *(retChar++) = ' ';                            // append a space
            }

        next1:
            currentNode = currentNode->next; // next node
        } while (currentNode);

        return SYSCALL_STATUS_OK;
    }

    case 3: // size of descriptor
    {
        return vfsSize(FD_TO_NODE(arg1));
    }

    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}