bits 64

section .text

global fpuInit

sseEnable:
    mov rax, cr0
    and ax, 0xFFFB ; clear CR0.EM
    or rax, 0x2 ; set CR0.MP
    mov cr0, rax

    mov rax, cr4
    or ax, 3 << 9 ;set CR4.OSFXSR and CR4.OSXMMEXCPT
    mov cr4, rax

    ret

avxEnable:
    ; check for support
    mov rax, 1
    cpuid
    and ecx, (1 << 28)

    jnz .enable ; if bit 28 is set then we can safely enable

    ret ; don't enable if not supported

.enable:
    mov rax, cr4
    or eax, 0x40000
    mov cr4, rax

    mov rcx, 0
    xgetbv

    or eax, 6
    mov rcx, 0
    xsetbv
    ret

fpuInit:
    fninit ; init x87
    
    call sseEnable
    call avxEnable

    ret ; return