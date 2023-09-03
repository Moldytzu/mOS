bits 64

%include "registers.inc"

global switchTo, saveSimdContextTo

; saves simd context
; rdi=context address
saveSimdContextTo:
    fxsave [rdi]
    ret

; switches to a new context
; rdi=stack context
; rsi=simd context
switchTo: 
    fxrstor [rsi] ; restore simd context

    mov rsp, rdi ; in rdi (first C argument) we store the interrupt stack's address
    POP_REG      ; restore registers
    iretq        ; restore flags, segments, stack and instruction pointer