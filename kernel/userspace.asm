bits 64

global sysretInit, userspaceJump

sysretInit:
	mov	rcx, 0xc0000080
	rdmsr
	or rax, 1
	wrmsr
	mov	rcx, 0xc0000081
	rdmsr
	mov	rdx, 0x00180008
	wrmsr
	ret

userspaceJump:
    mov rcx, rdi ; set the new rip
    mov rsp, rsi ; set the new stack
    mov r11, 0x202 ; rflags, enable intrerrupts
    o64 sysret ; to userspace and beyond