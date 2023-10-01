#pragma once
#include <misc/utils.h>

typedef struct
{
    uint32_t sender; // sender's pid
    size_t subject;  // numeric subject

    size_t messageLength; // length of message
    char message[];       // message
} mail_t;

typedef struct
{
    void *next;
    spinlock_t lock;

    size_t contentsPages;
    mail_t *contents;
} mailbox_t;

mailbox_t *mailCompose(mailbox_t *mailbox, uint32_t sender, size_t subject, char *message, size_t messageLength);
mailbox_t *mailReadNext(mailbox_t *mailbox);
void mailFreeContents(mailbox_t *mail);
void mailFreeBox(mailbox_t *mail);
void mailFree(mailbox_t *mail);