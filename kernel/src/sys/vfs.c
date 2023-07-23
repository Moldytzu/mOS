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
void vfs(uint64_t call, uint64_t arg1, uint64_t retVal, uint64_t r9, sched_task_t *task)
{
    if (!IS_MAPPED(retVal)) // prevent crashing
        return;

    struct vfs_node_t *currentNode;
    uint64_t *retAddr = PHYSICAL(retVal);

    switch (call)
    {
    case 0: // file path exists
    {
        if (!IS_MAPPED(arg1))
        {
            *retAddr = 0;
            return;
        }

        uint64_t fd = vfsOpen(PHYSICAL(arg1)); // open
        *retAddr = fd > 0;                     // if the fd is valid then the file exists
        vfsClose(fd);                          // close
        break;
    }
    case 1: // directory path exists
    {
        if (!IS_MAPPED(arg1))
        {
            *retAddr = 0;
            return;
        }

        const char *target = PHYSICAL(arg1); // target directory
        char fullPath[512];                  // full path of node

        currentNode = vfsNodes();
        do // iterate over all nodes
        {
            if (!currentNode->filesystem)
                goto next;

            vfsGetPath((uint64_t)currentNode, fullPath);

            if (strcmp(target, fullPath) == 0) // compare full paths
            {
                *retAddr = true; // found it
                return;
            }

        next:
            currentNode = currentNode->next; // next node
        } while (currentNode);

        *retAddr = false; // doesn't exist

        break;
    }
    case 2: // list directory
    {
        if (!IS_MAPPED(arg1))
        {
            *retAddr = 0;
            return;
        }

        const char *target = PHYSICAL(arg1); // target directory to list
        char *retChar = (char *)retAddr;     // return array

        char fullPath[512]; // full path of node

        logDbg(LOG_SERIAL_ONLY, "vfs: listing directory for %s", target);

        if (strlen(target) < 1) // invalid path
            break;

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
                return;
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

        break;
    }
    case 3: // size of descriptor
    {
        *retAddr = vfsSize(arg1);
        break;
    }
    default:
        break;
    }
}