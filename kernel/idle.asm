bits 64
align 4096

global idleTask

idleTask:
    mov rdi, 90
    syscall
    int 0x51
    syscall
    int 0x51
    jmp $ ; stay there