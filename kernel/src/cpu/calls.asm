bits 64

section .text

global callWithPageTable, callWithStack

; calls a function with a new page table
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
	ret ; return

callWithStack:
	mov rsp, rsi
	call rdi
	jmp $