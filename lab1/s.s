
         .global main, mymain, myprintf
main:    
         pushl  %ebp
         movl   %esp, %ebp
	
# (1). Write ASSEMBLY code to call myprintf(FMT)
#      HELP: How does mysum() call printf() in the class notes.

	pushl $FMT
	call myprintf
	

# (2). Write ASSEMBLY code to call mymain(argc, argv, env)
#      HELP: When crt0.o calls main(int argc, char *argv[], char *env[]), 
#            it passes argc, argv, env to main(). 
#            Draw a diagram to see where are argc, argv, env?

	addl $12,%esp
	call mymain
	


# (3). Write code to call myprintf(fmt,a,b)	
#      HELP: same as in (1) above

        pushl b
        pushl a 
        pushl $fmt

        call  myprintf

	#addl  $22,%esp
	#call  mymain

# (4). Return to caller
        movl  %ebp, %esp
	popl  %ebp
	ret

#---------- DATA section of assembly code ---------------
	.data
FMT:	.asciz "main() in assembly call mymain() in C\n"
a:	.long 1234
b:	.long 5678
fmt:	.asciz "a=%d b=%d\n"
#---------  end of s.s file ----------------------------


/********************* t.c file *********************************

mymain(int argc, char *argv[ ], char *env[ ])
{
  int i;

  myprintf("in mymain(): argc=%d\n", argc);

  for (i=0; i < argc; i++)
      myprintf("argv[%d] = %s\n", i, argv[i]);
  
  // WRITE CODE TO PRINT THE env strings

  myprintf("---------- testing YOUR myprintf() ---------\n");
  myprintf("this is a test\n");
  myprintf("testing a=%d b=%x c=%c s=%s\n", 123, 123, 'a', "testing");
  myprintf("string=%s, a=%d  b=%u  c=%o  d=%x\n",
           "testing string", -1024, 1024, 1024, 1024);
  myprintf("mymain() return to main() in assembly\n"); 
}*/
