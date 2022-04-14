bits 64

global sysretInit, userspaceJump

sysretInit:
	mov	rcx, 0xc0000080 ; IA32_EFER
	rdmsr
	or rax, 1 ; set SCE (syscall extensions)
	wrmsr
	mov	rcx, 0xc0000081 ; STAR
	rdmsr
	mov	rdx, 0x00100008 ; kbase is 0x00, ubase is 0x10
	wrmsr
	ret

userspaceJump:
    mov rcx, rdi ; set the new rip
    mov rsp, rsi ; set the new stack
    mov r11, 0x202 ; rflags, enable intrerrupts
    o64 sysret ; to userspace and beyond