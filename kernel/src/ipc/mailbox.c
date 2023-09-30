#include <ipc/mailbox.h>
#include <mm/pmm.h>
#include <mm/blk.h>

#define NEXT_OF(x) ((mailbox_t *)(x->next))
#define MAILBOX_EMPTY(x) (!(x->next))

// composes new mail then adds it in the mailbox
mailbox_t *mailCompose(mailbox_t *mailbox, uint32_t sender, size_t subject, char *message, size_t messageLength)
{
    mailbox_t *mail = mailbox;

    // get last valid mail
    while (mail && NEXT_OF(mail))
        mail = NEXT_OF(mail);

    // do the allocation
    lock(mail->lock, {
        mail->next = blkBlock(sizeof(mailbox_t)); // allocate the mailbox in the linked list
    });

    mail = NEXT_OF(mail); // point to the newly allocated mail

    mail->contentsPages = align(messageLength + sizeof(mail_t), PMM_PAGE) / PMM_PAGE; // calculate required pages to be allocated
    mail->contents = pmmPages(mail->contentsPages);                                   // allocate mail context

    // set required metadata
    mail->contents->sender = sender;
    mail->contents->subject = subject;
    mail->contents->messageLength = messageLength;
    memcpy(mail->contents->message, message, messageLength); // copy message

    return mail; // return the mail
}

// returns oldest mail and removes it from the chain
mailbox_t *mailReadNext(mailbox_t *mailbox)
{
    // oldest mail is always the first after the mailbox
    if (!NEXT_OF(mailbox)) // there isn't any mail left
        return NULL;

    mailbox_t *mail;
    lock(mailbox->lock, {
        mail = NEXT_OF(mailbox);    // mail to be returned
        mailbox->next = mail->next; // remove from linked list
    });

    return mail; // return it
}

// frees resources of a mail
void mailFree(mailbox_t *mail)
{
    if (!mail)
        return;

    pmmDeallocatePages(mail->contents, mail->contentsPages); // deallocate contents
    blkDeallocate(mail);                                     // deallocate metadata
}