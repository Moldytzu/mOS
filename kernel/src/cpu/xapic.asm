bits 64

section .text

%include "registers.inc"

global xapicEntry
extern xapicHandleTimer

; this is where the cpu jumps when the xapic timer fires
xapicEntry:
    PUSH_REG              ; save all general purpose registers + cr3
    mov rdi, rsp          ; pass the stack frame
    call xapicHandleTimer ; handle the timer
    POP_REG               ; restore registers
    iretq                 ; return to context