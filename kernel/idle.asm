bits 64
align 4096

global idleTask

idleTask:
    mov rdi, 0xFF
    int 0x51
    jmp $ ; don't do anything