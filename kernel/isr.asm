bits 64

%macro    PUSH_REG    0
    push    r15
    push    r14
    push    r13
    push    r12
    push    r11
    push    r10
    push    r9
    push    r8
    push    rbp
    push    rdi
    push    rsi
    push    rsp
    push    rdx
    push    rcx
    push    rbx
    push    rax
%endmacro
%macro    POP_REG        0
    pop    rax
    pop    rbx
    pop    rcx
    pop    rdx
    pop    rsp
    pop    rsi
    pop    rdi
    pop    rbp
    pop    r8
    pop    r9
    pop    r10
    pop    r11
    pop    r12
    pop    r13
    pop    r14
    pop    r15
%endmacro

global BaseHandlerEntry, PITHandlerEntry, SyscallHandlerEntry
extern BaseHandler, PITHandler, syscallHandler

BaseHandlerEntry:
    cli ; disable intrerrupts
    cld ; because of the ABI
    PUSH_REG
    call BaseHandler
    POP_REG
    sti ; enable intrerrupts
    iretq

PITHandlerEntry:
    cli ; disable intrerrupts
    cld ; because of the ABI
    PUSH_REG
    mov rdi, rsp ; give the handler the stack frame
    call PITHandler
    POP_REG
    sti ; enable intrerrupts
    iretq

SyscallHandlerEntry:
    cld ; because of the ABI
    PUSH_REG
    call syscallHandler ; call the syscall handler
    POP_REG
    iretq