#include <subsys/socket.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/pmm.h>

struct sock_socket rootSocket;
uint32_t lastSockID = 0;

// todo: negociate the buffer size

// create new socket and return it's address
struct sock_socket *sockCreate()
{
    struct sock_socket *currentSocket = &rootSocket; // first socket

    if (currentSocket->buffer) // check if the root socket is valid
    {
        while (currentSocket->next) // get last socket
            currentSocket = currentSocket->next;

        if (currentSocket->buffer)
        {
            currentSocket->next = pmmPage(sizeof(struct sock_socket)); // allocate next socket if the current socket is valid
            currentSocket->next->previous = currentSocket;             // set the previous socket
            currentSocket = currentSocket->next;                       // set current socket to the newly allocated socket
        }
    }

    zero(currentSocket, sizeof(struct sock_socket)); // clear the socket
    currentSocket->buffer = pmmPage();               // allocate the buffer
    zero((void *)currentSocket->buffer, VMM_PAGE);   // clear the buffer
    currentSocket->id = lastSockID++;                // set the ID

#ifdef K_SOCK_DEBUG
    printks("sock: creating new socket with ID %d\n\r", currentSocket->id);
#endif

    return currentSocket;
}

// append text on a socket
void sockAppend(struct sock_socket *sock, const char *str, size_t count)
{
    if (!sock || !count)
        return;

    const char *input = str;                 // input buffer
    if (sock->bufferIdx + count >= VMM_PAGE) // check if we could overflow
    {
        input += (sock->bufferIdx + count) - VMM_PAGE; // move the pointer until where it overflows
        count -= (sock->bufferIdx + count) - VMM_PAGE; // decrease the count by the number of bytes where it overflows
        zero((void *)sock->buffer, VMM_PAGE);          // clear the buffer
        sock->bufferIdx = 0;                           // reset the index
    }

    memcpy8((void *)((uint64_t)sock->buffer + sock->bufferIdx), (void *)input, count); // copy the buffer
    sock->bufferIdx += count;                                                          // increment the index

#ifdef K_SOCK_DEBUG
    printks("sock: appended %d bytes to socket %d\n\r", count, sock->id);
#endif
}

// read text from the socket
void sockRead(struct sock_socket *sock, const char *str, size_t count)
{
    if (!sock)
        return;

    memcpy((void *)str, (void *)sock->buffer, count);               // copy the buffer
    memmove((void *)sock->buffer, sock->buffer + count - 1, count); // move the content after the requested count at the front
    zero((void *)sock->buffer + count - 1, 4096 - count + 1);       // clear the ghost of the content
    sock->bufferIdx = 0;                                            // reset the index
}

// get first socket
struct sock_socket *sockRoot()
{
    return &rootSocket;
}

// get socket with id
struct sock_socket *sockGet(uint32_t id)
{
    if (id > lastSockID) // out of bounds
        return NULL;

    struct sock_socket *socket = &rootSocket; // first socket
    while (socket->id != id && socket->next)
        socket = socket->next;

    if (socket->id != id) // another sanity check
        return NULL;

    return socket;
}

// initialize the subsystem
void sockInit()
{
    zero(&rootSocket, sizeof(struct sock_socket));
    sockCreate(); // create the first socket

    printk("sock: created the first socket\n");
}

// free the socket
void sockDestroy(struct sock_socket *sock)
{
    sock->previous->next = sock->next;   // bypass this socket
    pmmDeallocate((void *)sock->buffer); // deallocate the buffer
    pmmDeallocate(sock);                 // free the socket
}