#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>

// We need to tell the stivale bootloader where we want our stack to be.
static uint8_t stack[8*1024*1024]; // 8 mb stack

// terminal
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID, // id
        .next = 0 // end
    },
    .flags = 0 // unusded
};

// framebuffer
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
        .next = (uint64_t)&terminal_hdr_tag
    },
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};

// stivale header
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
    .entry_point = 0, // elf entry point
    .stack = (uintptr_t)stack + sizeof(stack), // stack
    .flags = 0b1111,
    .tags = (uintptr_t)&framebuffer_hdr_tag // root of the linked list
};

// get tag
void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) {
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    while(1) {
        // check if the list is over
        if (current_tag == NULL) return NULL;

        // check whether the identifier matches.
        if (current_tag->identifier == id) return current_tag;

        // get the next tag
        current_tag = (void *)current_tag->next;
    }
}

// entry point of the kernel
void _start(struct stivale2_struct *stivale2_struct) {
    // get terminal
    struct stivale2_struct_tag_terminal *term_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);

    // check if the tag was actually found.
    if (term_str_tag == NULL) {
        // if it wasn't found, just hang...
        for (;;) {
            asm ("hlt");
        }
    }

    // get address of the function
    void *term_write_ptr = (void *)term_str_tag->term_write;

    // assign it to a function def
    void (*term_write)(const char *string, size_t length) = term_write_ptr;

    // display "Hello World"
    term_write("Hello World", 11);

    // We're done, just hang...
    for (;;) {
        asm ("hlt");
    }
}
