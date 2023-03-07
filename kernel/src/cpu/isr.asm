bits 64

%macro PUSH_REG 0
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
    mov rax, cr3
    push rax
%endmacro

%macro POP_REG 0
    pop rax
    mov cr3, rax
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
%endmacro

%macro GEN_HANDLER 1
global BaseHandlerEntry%1

BaseHandlerEntry%1:
    cli ; disable intrerrupts
    PUSH_REG
    mov rdi, rsp ; give the handler the stack frame
    mov rsi, %1 ; give the intrerrupt number
    call exceptionHandler
    POP_REG
    iretq
%endmacro

global lapicEntry, SyscallHandlerEntry
extern PITHandler, syscallHandler, ps2Port1Handler, ps2Port2Handler, exceptionHandler

%assign i 0
%rep 256
GEN_HANDLER i
%assign i i+1
%endrep

SyscallHandlerEntry:
    push rax ; simulate error push
    PUSH_REG
    mov rdi, rsp
    call syscallHandler ; call the syscall handler
    POP_REG
    add rsp, 8 ; hide that push
    o64 sysret ; return to userspace

lapicEntry:
    cli ; disable intrerrupts
    push rax ; simulate error push
    PUSH_REG
    mov rdi, rsp ; give the handler the stack frame
    mov rsi, 0x20 ; give the intrerrupt number
    call exceptionHandler
    POP_REG
    add rsp, 8 ; hide push
    iretq

section .data
int_table:
%assign i 0
%rep 256
    dq BaseHandlerEntry%+i
%assign i i+1
%endrep
[GLOBAL int_table]