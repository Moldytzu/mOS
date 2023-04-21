#pragma once
#include <misc/utils.h>

#define SOCK_SIZE (1 * 4096)

struct sock_socket
{
    uint32_t id;   // ID
    int bufferIdx; // current index in the buffer
    int readIdx;   // current read index in the buffer

    struct sock_socket *previous; // previous terminal
    struct sock_socket *next;     // next terminal

    uint8_t buffer[]; // data to be used
};

#define SOCK_BUFFER_SIZE (SOCK_SIZE - offsetof(struct sock_socket, buffer))

void sockDestroy(struct sock_socket *sock);
void sockInit();
struct sock_socket *sockCreate();
void sockAppend(struct sock_socket *sock, const char *str, size_t count);
void sockRead(struct sock_socket *sock, const char *str, size_t count);
struct sock_socket *sockGet(uint32_t id);
struct sock_socket *sockRoot();