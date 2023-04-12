bits 64

section .text

global sysretInit, callWithPageTable, callWithStack
extern SyscallHandlerEntry

; initialize the sysret/syscall functionality
sysretInit:
	mov rax, SyscallHandlerEntry ; syscall handler entry
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

callWithPageTable:
	; set up the stack
	push rbp
	mov rbp, rsp
	
	mov r11, cr3 
	push r11     ; push the old page table
	mov cr3, rsi ; set new page table
	call rdi     ; call the function
	pop r11
	mov cr3, r11 ; set the old page table

	; restore the stack
	mov rsp, rbp  
	pop rbp       
	ret          ; return

callWithStack:
	mov rsp, rsi
	call rdi
	jmp $