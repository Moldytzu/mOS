#pragma once
#include <misc/utils.h>

typedef struct
{
    void *next;

    locker_t lock; // spinlock

    uint32_t sender; // sender's pid
    size_t subject;  // numeric subject

    size_t messageLength; // length of message
    char message[];       // message
} mailbox_t;

mailbox_t *mailCompose(mailbox_t *mailbox, uint32_t sender, size_t subject, char *message, size_t messageLength);
mailbox_t *mailReadNext(mailbox_t *mailbox);
void mailFree(mailbox_t *mail);