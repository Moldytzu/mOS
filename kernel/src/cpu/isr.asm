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
global __baseHandler%1

__baseHandler%1:
    cld                   ; clear direction flag as the sysv abi mandates
    cli                   ; disable intrerrupts
    PUSH_REG              ; save old registers
    mov rdi, rsp          ; give the handler the stack frame
    mov rsi, %1           ; give the intrerrupt number
    call exceptionHandler ; call the handler
    POP_REG               ; restore registers
    iretq                 ; return to previous context
%endmacro

global lapicEntry, syscallHandlerEntry, int_table
extern syscallHandler, exceptionHandler, lapicHandleTimer

%assign i 0
%rep 256
GEN_HANDLER i
%assign i i+1
%endrep

syscallHandlerEntry:
    cld                 ; clear direction flag as the sysv abi mandates
    push rax            ; push an arbitrary error code
    PUSH_REG            ; save old registers
    mov rdi, rsp        ; point to the stack frame
    call syscallHandler ; call the syscall handler
    POP_REG             ; restore registers
    pop rax             ; pop the error code from earlier
    o64 sysret          ; return to userspace

lapicEntry:
    cld                   ; clear direction flag as the sysv abi mandates
    cli                   ; disable intrerrupts
    push rax              ; push an arbitrary error code
    PUSH_REG              ; save all general purpose registers + cr3
    mov rdi, rsp          ; pass the stack frame
    call lapicHandleTimer ; handle the timer
    POP_REG               ; restore registers
    pop rax               ; pop the error code from earlier
    iretq                 ; return to context

section .data
int_table:
%assign i 0
%rep 256
    dq __baseHandler%+i
%assign i i+1
%endrep