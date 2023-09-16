#include <ipc/socket.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/blk.h>
#include <misc/logger.h>

struct sock_socket rootSocket;
uint32_t lastSockID = 1;

// todo: permission system (the socket creator can chose which pids can read/write from the socket)
// adjustable buffer size (dynamically allocate buffer size)
// master-slave system (multiple connections on the same socket, create a buffer for each connection)

// create new socket and return it's address
struct sock_socket *sockCreate()
{
    struct sock_socket *currentSocket = &rootSocket; // first socket

    while (currentSocket->next) // point to the last socket
        currentSocket = currentSocket->next;

    currentSocket->next = pmmPage();               // allocate next socket if the current socket is valid
    currentSocket->next->previous = currentSocket; // set the previous socket

    currentSocket = currentSocket->next; // point to the new socket
    currentSocket->id = lastSockID++;    // set the ID

#ifdef K_SOCK_DEBUG
    logDbg(LOG_SERIAL_ONLY, "sock: creating new socket with ID %d\n\r", currentSocket->id);
#endif

    return currentSocket;
}

// append text on a socket
void sockAppend(struct sock_socket *sock, const char *str, size_t count)
{
    if (!sock || !count)
        return;

    lock(sock->lock, {
        const char *input = str;                         // input buffer
        if (sock->bufferIdx + count >= SOCK_BUFFER_SIZE) // check if we could overflow
        {
            input += (sock->bufferIdx + count) - SOCK_BUFFER_SIZE; // move the pointer until where it overflows
            count -= (sock->bufferIdx + count) - SOCK_BUFFER_SIZE; // decrease the count by the number of bytes where it overflows
            zero((void *)sock->buffer, SOCK_BUFFER_SIZE);          // clear the buffer
            sock->bufferIdx = 0;                                   // reset the index
        }

        memcpy8((void *)((uint64_t)sock->buffer + sock->bufferIdx), (void *)input, count); // copy the buffer
        sock->bufferIdx += count;                                                          // increment the index
    });

#ifdef K_SOCK_DEBUG
    logDbg(LOG_SERIAL_ONLY, "sock: appended %d bytes to socket %d\n\r", count, sock->id);
#endif
}

// read text from the socket
void sockRead(struct sock_socket *sock, const char *str, size_t count)
{
    if (!sock)
        return;

    lock(sock->lock, {
        count = min(count, SOCK_BUFFER_SIZE - 1); // don't overflow

        memcpy((void *)str, (void *)sock->buffer, count); // copy the wanted buffer

        // shift buffer left
        int k = 0;
        int i = count;
        while (i < SOCK_BUFFER_SIZE)
            sock->buffer[k++] = sock->buffer[i++];

        // decrement write pointer if possible
        if (sock->bufferIdx > count)
            sock->bufferIdx = sock->bufferIdx - count;
        else
            sock->bufferIdx = 0;
    });
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
}

// free the socket
void sockDestroy(struct sock_socket *sock)
{
    sock->previous->next = sock->next; // bypass this socket
    pmmDeallocate(sock);               // free the socket
}