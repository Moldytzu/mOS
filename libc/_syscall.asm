bits 64

global _syscall

_syscall:
    mov r8,rcx
    syscall
    ret