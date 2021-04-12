	.file	"t.c"
	.globl	BASE
	.data
	.align 4
	.type	BASE, @object
	.size	BASE, 4
BASE:
	.long	10
	.globl	table
	.section	.rodata
.LC0:
	.string	"0123456789ABCDEF"
	.data
	.align 4
	.type	table, @object
	.size	table, 4
table:
	.long	.LC0
	.text
	.globl	rpu
	.type	rpu, @function
rpu:
.LFB2:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%esi
	pushl	%ebx
	subl	$32, %esp
	.cfi_offset 6, -12
	.cfi_offset 3, -16
	cmpl	$0, 8(%ebp)
	je	.L3
	movl	table, %ecx
	movl	12(%ebp), %ebx
	movl	8(%ebp), %eax
	movl	$0, %edx
	divl	%ebx
	movl	%edx, %eax
	addl	%ecx, %eax
	movzbl	(%eax), %eax
	movb	%al, -9(%ebp)
	movl	12(%ebp), %esi
	movl	8(%ebp), %eax
	movl	$0, %edx
	divl	%esi
	movl	12(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	rpu
	movsbl	-9(%ebp), %eax
	movl	%eax, (%esp)
	call	putchar
.L3:
	addl	$32, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.size	rpu, .-rpu
	.globl	printu
	.type	printu, @function
printu:
.LFB3:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	cmpl	$0, 8(%ebp)
	jne	.L5
	movl	$48, (%esp)
	call	putchar
	jmp	.L6
.L5:
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	rpu
.L6:
	movl	$32, (%esp)
	call	putchar
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE3:
	.size	printu, .-printu
	.globl	prints
	.type	prints, @function
prints:
.LFB4:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	$0, -12(%ebp)
	jmp	.L8
.L9:
	movl	-12(%ebp), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	movsbl	%al, %eax
	movl	%eax, (%esp)
	call	putchar
	addl	$1, -12(%ebp)
.L8:
	movl	-12(%ebp), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	testb	%al, %al
	jne	.L9
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE4:
	.size	prints, .-prints
	.globl	printd
	.type	printd, @function
printd:
.LFB5:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	cmpl	$0, 8(%ebp)
	jne	.L11
	movl	$48, (%esp)
	call	putchar
	jmp	.L14
.L11:
	cmpl	$0, 8(%ebp)
	jle	.L13
	movl	8(%ebp), %eax
	movl	$10, 4(%esp)
	movl	%eax, (%esp)
	call	printu
	jmp	.L14
.L13:
	movl	$45, (%esp)
	call	putchar
	movl	8(%ebp), %eax
	negl	%eax
	movl	$10, 4(%esp)
	movl	%eax, (%esp)
	call	printu
.L14:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE5:
	.size	printd, .-printd
	.globl	printo
	.type	printo, @function
printo:
.LFB6:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	$48, (%esp)
	call	putchar
	cmpl	$0, 8(%ebp)
	jne	.L16
	movl	$48, (%esp)
	call	putchar
	jmp	.L19
.L16:
	cmpl	$0, 8(%ebp)
	je	.L18
	movl	$8, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	printu
	jmp	.L19
.L18:
	movl	$45, (%esp)
	call	putchar
	movl	8(%ebp), %eax
	negl	%eax
	movl	$8, 4(%esp)
	movl	%eax, (%esp)
	call	printu
.L19:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE6:
	.size	printo, .-printo
	.globl	printx
	.type	printx, @function
printx:
.LFB7:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	$48, (%esp)
	call	putchar
	movl	$120, (%esp)
	call	putchar
	cmpl	$0, 8(%ebp)
	jne	.L21
	movl	$48, (%esp)
	call	putchar
	jmp	.L24
.L21:
	cmpl	$0, 8(%ebp)
	je	.L23
	movl	$16, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	printu
	jmp	.L24
.L23:
	movl	$45, (%esp)
	call	putchar
	movl	8(%ebp), %eax
	negl	%eax
	movl	$16, 4(%esp)
	movl	%eax, (%esp)
	call	printu
.L24:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE7:
	.size	printx, .-printx
	.globl	myprintf
	.type	myprintf, @function
myprintf:
.LFB8:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	8(%ebp), %eax
	movl	%eax, -12(%ebp)
	leal	12(%ebp), %eax
	movl	%eax, -20(%ebp)
	movl	$0, -16(%ebp)
	jmp	.L26
.L38:
	movl	8(%ebp), %edx
	movl	-16(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	cmpb	$37, %al
	jne	.L27
	addl	$1, -16(%ebp)
	movl	8(%ebp), %edx
	movl	-16(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	movb	%al, -21(%ebp)
	movsbl	-21(%ebp), %eax
	subl	$99, %eax
	cmpl	$21, %eax
	ja	.L28
	movl	.L30(,%eax,4), %eax
	jmp	*%eax
	.section	.rodata
	.align 4
	.align 4
.L30:
	.long	.L29
	.long	.L31
	.long	.L28
	.long	.L28
	.long	.L28
	.long	.L28
	.long	.L28
	.long	.L28
	.long	.L28
	.long	.L28
	.long	.L28
	.long	.L28
	.long	.L32
	.long	.L28
	.long	.L28
	.long	.L28
	.long	.L33
	.long	.L28
	.long	.L34
	.long	.L28
	.long	.L28
	.long	.L35
	.text
.L29:
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, (%esp)
	call	putchar
	jmp	.L36
.L33:
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, (%esp)
	call	prints
	jmp	.L36
.L34:
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	movl	$10, 4(%esp)
	movl	%eax, (%esp)
	call	printu
	jmp	.L36
.L31:
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, (%esp)
	call	printd
	jmp	.L36
.L32:
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, (%esp)
	call	printo
	jmp	.L36
.L35:
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, (%esp)
	call	printx
	jmp	.L36
.L28:
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, (%esp)
	call	putchar
	nop
.L36:
	addl	$4, -20(%ebp)
	jmp	.L37
.L27:
	movl	8(%ebp), %edx
	movl	-16(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	movsbl	%al, %eax
	movl	%eax, (%esp)
	call	putchar
.L37:
	addl	$1, -16(%ebp)
.L26:
	movl	8(%ebp), %edx
	movl	-16(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	testb	%al, %al
	jne	.L38
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE8:
	.size	myprintf, .-myprintf
	.section	.rodata
.LC1:
	.string	"in mymain(): argc=%d\n"
.LC2:
	.string	"argv[%d] = %s\n"
	.align 4
.LC3:
	.string	"---------- testing YOUR myprintf() ---------\n"
.LC4:
	.string	"this is a test\n"
.LC5:
	.string	"testing"
.LC6:
	.string	"testing a=%d b=%x c=%c s=%s\n"
.LC7:
	.string	"testing string"
	.align 4
.LC8:
	.string	"string=%s, a=%d  b=%u  c=%o  d=%x\n"
	.align 4
.LC9:
	.string	"mymain() return to main() in assembly\n"
	.text
	.globl	mymain
	.type	mymain, @function
mymain:
.LFB9:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$56, %esp
	movl	8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	myprintf
	movl	$0, -12(%ebp)
	jmp	.L40
.L41:
	movl	-12(%ebp), %eax
	leal	0(,%eax,4), %edx
	movl	12(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	movl	%eax, 8(%esp)
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC2, (%esp)
	call	myprintf
	addl	$1, -12(%ebp)
.L40:
	movl	-12(%ebp), %eax
	cmpl	8(%ebp), %eax
	jl	.L41
	movl	$.LC3, (%esp)
	call	myprintf
	movl	$.LC4, (%esp)
	call	myprintf
	movl	$.LC5, 16(%esp)
	movl	$97, 12(%esp)
	movl	$123, 8(%esp)
	movl	$123, 4(%esp)
	movl	$.LC6, (%esp)
	call	myprintf
	movl	$1024, 20(%esp)
	movl	$1024, 16(%esp)
	movl	$1024, 12(%esp)
	movl	$-1024, 8(%esp)
	movl	$.LC7, 4(%esp)
	movl	$.LC8, (%esp)
	call	myprintf
	movl	$.LC9, (%esp)
	call	myprintf
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE9:
	.size	mymain, .-mymain
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
