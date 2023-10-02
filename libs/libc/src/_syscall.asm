bits 64

global _syscall

_syscall:
    ; this is a weird way of calling the system
    ; the syscall type is in rdi
    ; and the parameters are in rsi (first), rdx (second), r8 (third), r9 (fourth) and r10 (fifth)
    
    ; now, this function has the first argument in rdi (the type)
    ; the second argument in rsi (first parameter)
    ; the third argument in rdx (second parameter)
    ; the fourth argument in rcx (third parameter)
    ; the fifth argument in r8 (fourth parameter)
    ; the sixth argument in r9 (fifth parameter)
    
    ; the rcx register will not be used by the syscall handler
    ; thus we have to move r9 in r10, r8 in r9, rcx in r8

    mov r10, r9
    mov r9, r8
    mov r8, rcx
    syscall ; this puts the return value in rax as sysv abi wants
    ret