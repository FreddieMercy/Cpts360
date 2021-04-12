	.file	"t1.c"
	.section	.rodata
.LC0:
	.string	"in mymain(): argc=%d\n"
.LC1:
	.string	"argv[%d] = %s\n"
	.align 4
.LC2:
	.string	"---------- testing YOUR myprintf() ---------\n"
.LC3:
	.string	"this is a test\n"
.LC4:
	.string	"testing"
.LC5:
	.string	"testing a=%d b=%x c=%c s=%s\n"
.LC6:
	.string	"testing string"
	.align 4
.LC7:
	.string	"string=%s, a=%d  b=%u  c=%o  d=%x\n"
	.align 4
.LC8:
	.string	"mymain() return to main() in assembly\n"
	.text
	.globl	mymain
	.type	mymain, @function
mymain:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$56, %esp
	movl	8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC0, (%esp)
	call	myprintf
	movl	$0, -12(%ebp)
	jmp	.L2
.L3:
	movl	-12(%ebp), %eax
	leal	0(,%eax,4), %edx
	movl	12(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	movl	%eax, 8(%esp)
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	myprintf
	addl	$1, -12(%ebp)
.L2:
	movl	-12(%ebp), %eax
	cmpl	8(%ebp), %eax
	jl	.L3
	movl	$.LC2, (%esp)
	call	myprintf
	movl	$.LC3, (%esp)
	call	myprintf
	movl	$.LC4, 16(%esp)
	movl	$97, 12(%esp)
	movl	$123, 8(%esp)
	movl	$123, 4(%esp)
	movl	$.LC5, (%esp)
	call	myprintf
	movl	$1024, 20(%esp)
	movl	$1024, 16(%esp)
	movl	$1024, 12(%esp)
	movl	$-1024, 8(%esp)
	movl	$.LC6, 4(%esp)
	movl	$.LC7, (%esp)
	call	myprintf
	movl	$.LC8, (%esp)
	call	myprintf
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	mymain, .-mymain
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
