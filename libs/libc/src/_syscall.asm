bits 64

global _syscall

_syscall:
    mov r9, r8
    mov r8, rcx
    syscall ; this puts the return value in rax as sysv abi wants
    ret