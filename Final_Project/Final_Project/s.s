        .globl  show, printf,DEBUG

show:
         movl DEBUG,%eax
	 cmpl $0,%eax
	 jz   return
	 popl %esi
	 call printf
	 pushl %esi
return:	
	 ret
	
