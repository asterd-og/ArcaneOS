[bits 64]
[extern interrupts_handle_int]
[global isr_table]

%macro pushaq 0
	push rax
	push rcx
	push rdx
	push rbx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popaq 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rbx
	pop rdx
	pop rcx
	pop rax
%endmacro

%macro int_stub 1
int_stub%+%1:
	%if !(%1 == 8 || (%1 >= 10 && %1 <= 14) || %1 == 17 || %1 == 21 || %1 == 29 || %1 == 30)
	push 0
	%endif
	push %1
	pushaq

	mov rdi, rsp
	call interrupts_handle_int

	popaq
	add rsp, 16
	iretq
%endmacro

%assign i 0
%rep 256
int_stub i
%assign i i + 1
%endrep

[section .data]

%assign j 0

isr_table:
	%rep 256
	dq int_stub%+j
	%assign j j + 1
	%endrep