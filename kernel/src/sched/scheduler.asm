bits 64

section .text

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
; rdx=new page table
switchTo: 
    fxrstor [rsi] ; restore simd context

    ; restore page table
    mov cr3, rdx

    mov rsp, rdi ; first argument holds the interrupt stack's address
    POP_REG      ; restore registers (fixme: here we might restore the page table a second time thus clearing the tlb which takes a lot of time)
    iretq        ; restore flags, segments, stack and instruction pointer