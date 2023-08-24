bits 64

%include "registers.inc"

global switchTo

switchTo:        ; switches to a new context
    mov rsp, rdi ; in rdi (first C argument) we store the interrupt stack's address
    POP_REG      ; restore registers
    iretq        ; restore flags, segments, stack and instruction pointer