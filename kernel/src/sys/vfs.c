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
    if (!INBOUNDARIES(retVal)) // prevent crashing
        return;

    struct vfs_node_t *currentNode;
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

        char path[512];

        logDbg(LOG_SERIAL_ONLY, "vfs: listing directory for %s", tmp);

        currentNode = vfsNodes();
        do
        {
            if (!currentNode->filesystem)
                goto next1;

            name = tmp; // reset the pointer

            zero(path, sizeof(path));
            vfsGetPath((uint64_t)currentNode, path);

            bool listFile = path[strlen(name) - 1] != '/';              // check if we list a file
            bool starts = strstarts(path, name);                        // check if path starts with the same characters
            bool hasSameSlashes = count(path, '/') == count(name, '/'); // check if the path and name has the same number of slashes
            bool endsInSlash = path[strlen(path) - 1] == '/';           // check if the path is a directory

            if (starts && (hasSameSlashes /* this prevents going in subdirectories */ || endsInSlash /* this lets only directories */) && !listFile)
            {
                if (strlen(currentNode->path) == 0 && !hasSameSlashes) // append mount name for partitions or nodes without path that aren't the one we search in
                {
                    memcpy(retChar, currentNode->filesystem->mountName + 1 /*mount name starts with /, maybe we want to cut the requested directory instead*/, strlen(currentNode->filesystem->mountName) - 1); // copy the path
                    retChar += strlen(currentNode->filesystem->mountName) - 1;                                                                                                                                  // move the pointer forward
                }
                else
                {
                    memcpy(retChar, currentNode->path, strlen(currentNode->path)); // copy the path
                    retChar += strlen(currentNode->path);                          // move the pointer forward
                }

                *(retChar++) = ' '; // append a space
            }
            else if (listFile && strcmp(path, name) == 0) // we list a file so we only show it
            {
                memcpy(retChar, currentNode->path, strlen(currentNode->path)); // copy the local path
                retChar += strlen(currentNode->path);                          // move the pointer forward
                break;
            }

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