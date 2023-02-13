bits 64

global smpID

smpID:
    mov rax, 1
    cpuid
    shr rbx, 24

    mov rax, rbx
    ret
