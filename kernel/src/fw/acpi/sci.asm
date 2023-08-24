bits 64

%include "registers.inc"

global sciEntry
extern sciHandler

; this is where the cpu jumps
sciEntry:
    cld             ; clear the direction flag
    cli             ; disable intrerrupts
    PUSH_REG        ; save all general purpose registers + cr3
    mov rdi, rsp    ; pass the stack frame
    call sciHandler ; handle the timer
    POP_REG         ; restore registers
    iretq           ; return to context