bits 64

global smpID

smpID:
    push rbx

    mov rax, 1
    cpuid
    shr rbx, 24

    mov rax, rbx
    pop rbx
    ret
