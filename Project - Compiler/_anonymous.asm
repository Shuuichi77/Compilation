extern printf
extern scanf

section .data
	printInt: db "%d", 10, 0
	printChar: db "%c", 10, 0
	readInt: db "%d", 0
	readChar: db "%c", 0

section .bss
	scanfInt: resd 1
	scanfChar: resb 1

section .text
global main

warning:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov ebx, 64
	mov eax, ebx
	mov rsp, rbp
	pop rbp
	ret

not_warning:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov bl, 'c'
	mov eax, ebx
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov bl, [rbp - 5]
	mov r12d, 64
	mov [rbp - 5], r12b
	mov ebx, [rbp - 4]
	mov r12b, 'c'
	mov [rbp - 4], r12d
	mov ebx, [rbp - 4]
	mov rdi, readInt
	mov rsi, scanfInt
	mov rax, 0
	call scanf
	mov ebx, [scanfInt]
	mov [rbp - 4], ebx
	mov bl, [rbp - 5]
	mov rdi, readChar
	mov rsi, scanfChar
	mov rax, 0
	call scanf
	mov bl, [scanfChar]
	mov [rbp - 5], bl
	mov ebx, 0
	mov eax, ebx
	mov rsp, rbp
	pop rbp
	ret
