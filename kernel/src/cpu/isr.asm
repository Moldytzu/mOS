bits 64

%macro PUSH_REG_SYSCALL 0
    ; save registers used by syscall instruction
    push r11
    push rcx

    ; save page table
    mov r11, cr3 ; use a scratch register to push cr3 (page table address)
    push r11
%endmacro

%macro POP_REG_SYSCALL 0
    ; reload page table
    pop r11 ; use the same scratch register to pop cr3
    mov cr3, r11
    
    ; load registers used by syscall instruction
    pop rcx
    pop r11
%endmacro

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
extern syscallHandler, exceptionHandler, xapicHandleTimer

%assign i 0
%rep 256
GEN_HANDLER i
%assign i i+1
%endrep

syscallHandlerEntry:    ; we start with flags cleared because of FMASK set in cpu/userspace.asm
    PUSH_REG_SYSCALL    ; save old registers
    call syscallHandler ; call the syscall handler
    POP_REG_SYSCALL     ; restore registers
    o64 sysret          ; return to userspace

lapicEntry:
    cli                   ; disable intrerrupts
    PUSH_REG              ; save all general purpose registers + cr3
    mov rdi, rsp          ; pass the stack frame
    call xapicHandleTimer ; handle the timer
    POP_REG               ; restore registers
    iretq                 ; return to context

section .data
int_table:
%assign i 0
%rep 256
    dq __baseHandler%+i
%assign i i+1
%endrep