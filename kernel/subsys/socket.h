#pragma once
#include <misc/utils.h>

struct sock_socket
{
    const char *buffer; // pointer to the terminal buffer
    int bufferIdx;      // current index in the buffer
    int readIdx;        // current read index in the buffer
    uint32_t id;        // ID

    struct sock_socket *previous; // previous terminal
    struct sock_socket *next;     // next terminal
};

void sockDestroy(struct sock_socket *sock);
void sockInit();
struct sock_socket *sockCreate();
void sockAppend(struct sock_socket *sock, const char *str, size_t count);
void sockRead(struct sock_socket *sock, const char *str, size_t count);
struct sock_socket *sockGet(uint32_t id);
struct sock_socket *sockRoot();