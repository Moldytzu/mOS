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

global BaseHandlerEntry, PITHandlerEntry, SyscallHandlerEntry, SyscallIntHandlerEntry, PS2Port1HandlerEntry, PS2Port2HandlerEntry
extern PITHandler, syscallHandler, ps2Port1Handler, ps2Port2Handler

BaseHandlerEntry:
    cli ; disable intrerrupts
    jmp $ ; don't do anything
    iretq

PITHandlerEntry:
    PUSH_REG
    mov rdi, rsp ; give the handler the stack frame
    call PITHandler
    POP_REG
    iretq

SyscallHandlerEntry:
    cli
    PUSH_REG
    call syscallHandler ; call the syscall handler
    POP_REG
    o64 sysret ; return to userspace

SyscallIntHandlerEntry:
    cli
    PUSH_REG
    call syscallHandler ; call the syscall handler
    POP_REG
    iretq ; terminate intrerrupt and return to userspace

PS2Port1HandlerEntry:
    PUSH_REG
    call ps2Port1Handler
    POP_REG
    iretq

PS2Port2HandlerEntry:
    PUSH_REG
    call ps2Port2Handler
    POP_REG
    iretq