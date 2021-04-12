#include <stdio.h>
#include <stdlib.h>

typedef unsigned int u32;
int BASE = 10;

char *table = "0123456789ABCDEF";

int rpu(u32 x, int base)
{
  char c;
  if (x){
     c = table[x % base];
     rpu(x / base, base);
     putchar(c);
  }
} 

int printu(u32 x, int base)
{
  if (x==0)
     putchar('0');
  else
     rpu(x, base);
  putchar(' ');
}

int prints(char *s)
{
	int i;
	for(i = 0; s[i] != '\0'; i++)
	{
		putchar(s[i]);
	}
}

int printd(int x)//: print an integer (which may be negative!!)
{
	if(x == 0)
	{
		putchar('0');
	}
	else if(x > 0)
	{
		printu(x, 10);
	}
	else
	{
		putchar('-');
		printu(~x +1, 10);
	}
}

int printo(u32 x)//: print x in OCTal as 0.....
{
	putchar('0');
	if(x == 0)
	{
		putchar('0');
	}
	else if(x > 0)
	{
		printu(x, 8);
	}
	else
	{
		putchar('-');
		printu(~x +1, 8);
	}	

}

int printx(u32 x)//: print x in HEX.  as 0x....
{
	putchar('0');
	putchar('x');
	if(x == 0)
	{
		putchar('0');
	}
	else if(x > 0)
	{
		printu(x, 16);
	}
	else
	{
		putchar('-');
		printu(~x +1, 16);
	}
}

int myprintf(char *fmt, ...) // SOME C compiler requires the 3 DOTs
{
	char *cp = fmt;
	int *ip = &fmt + 1;
	int i;
	for(i = 0; fmt[i] != '\0'; i++)
	{
		if(fmt[i] == '%')
		{
			i++;
			char temp = fmt[i];
						    
			switch(temp)
			{
				case 'c':
					putchar(*ip);
					break;
				case 's':
					prints(*ip);
					break;
				case 'u':
					printu(*ip, 10);
					break;
				case 'd':
					printd(*ip);
					break;
				case 'o':
					printo(*ip);
					break;
				case 'x':
					printx(*ip);
					break;
				default:
					putchar(*ip);
					break;
			}
			ip++;			
		}
		else
		{
			putchar(fmt[i]);
		}
	}
}


int mymain(int argc, char *argv[ ], char *env[ ])
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
}

