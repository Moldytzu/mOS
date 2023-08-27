#include <ipc/mailbox.h>
#include <mm/pmm.h>

#define NEXT_OF(x) ((mailbox_t *)(x->next))
#define MAILBOX_EMPTY(x) (!(x->next))

// composes new mail then adds it in the mailbox
mailbox_t *mailCompose(mailbox_t *mailbox, uint32_t sender, size_t subject, char *message, size_t messageLength)
{
    mailbox_t *mail = mailbox;

    // if the mailbox is not empty get last valid mail from it
    if (!MAILBOX_EMPTY(mail))
    {
        while (mail && NEXT_OF(mail))
            mail = NEXT_OF(mail);
    }

    // allocate next mail in the mailbox's linked list
    lock(mail->lock, {
        size_t requiredPages = (messageLength + sizeof(mailbox_t)) / 4096 + 1; // calculate required pages to be allocated
        mail->next = pmmPages(requiredPages);                                  // allocate next mail in the list
    });

    mail = NEXT_OF(mail); // point to the newly allocated mail

    // set required metadata
    mail->sender = sender;
    mail->subject = subject;
    mail->messageLength = messageLength;
    memcpy(mail->message, message, messageLength); // copy message

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

    size_t usedPages = (mail->messageLength + sizeof(mailbox_t)) / 4096 + 1; // calculate used pages
    pmmDeallocatePages(mail, usedPages);                                     // do the deallocation
}