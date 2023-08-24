bits 64

section .text

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

global sysretInit, sysretInit
extern syscallHandler

; initialize the sysret/syscall functionality
sysretInit:
	mov rax, syscallHandlerEntry ; syscall handler entry
	mov	rdx, rax
	shr	rdx, 0x20
	mov rcx, 0xC0000082 ; LSTAR
	wrmsr
	mov	rcx, 0xc0000080 ; IA32_EFER
	rdmsr
	or eax, 1 ; set SCE (syscall extensions)
	wrmsr
	mov	rcx, 0xc0000081 ; STAR
	rdmsr
	mov	rdx, 0x00130008 ; syscall base is 0x08, sysret base is 0x13
	wrmsr
	mov	rcx, 0xc0000084 ; FMASK
	rdmsr
	mov	rax, (0xFFFFFFFFFFFFFFFF & ~0b10) ; mask every flag
	wrmsr
	ret

; this is where the cpu jumps when we execute the syscall instruction
syscallHandlerEntry:    ; we start with flags cleared because of FMASK set in cpu/userspace.asm
    PUSH_REG_SYSCALL    ; save old registers
    call syscallHandler ; call the syscall handler
    POP_REG_SYSCALL     ; restore registers
    o64 sysret          ; return to userspace
