#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
              
#define MAX 10000
#define BLKSIZE 4096

char *cmd[] = {"cp", "mkdir", "rmdir", "ls", "cd", "cat", "pwd", "rm",
               "quit", "help", "?",  "reload", "save", 0};
FILE *fp;//, *gp;
int fd, gd;
char buf[4096];

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

struct stat mystat, *sp;

typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];

int findCmd(char *command)
{
   int i = 0;
   while(cmd[i]){
     if (strcmp(command, cmd[i])==0)
         return i;
     i++;
   }
   return -1;
}

int ls_file(char *fname)
{
  printf("<p>");
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];

  sp = &fstat;
  //printf("name=%s\n", fname); getchar();

  if ( (r = lstat(fname, &fstat)) < 0){
     printf("can't stat %s\n", fname); 
     exit(1);
  }

  if ((sp->st_mode & 0xF000) == 0x8000)
     printf("%c",'-');
  if ((sp->st_mode & 0xF000) == 0x4000)
     printf("%c",'d');
  if ((sp->st_mode & 0xF000) == 0xA000)
     printf("%c",'l');

  for (i=8; i >= 0; i--){
    if (sp->st_mode & (1 << i))
	printf("%c", t1[i]);
    else
	printf("%c", t2[i]);
  }

  printf("%4d ",sp->st_nlink);
  printf("%4d ",sp->st_gid);
  printf("%4d ",sp->st_uid);
  printf("%8d ",sp->st_size);

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  printf("%s  ",ftime);

  // print name
  printf("%s", basename(fname));  

  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000)
  { 
	
	   char *linkname;

	   linkname = malloc(sp->st_size + 1);

	    if (linkname == NULL) 
	    {
		fprintf(stderr, "insufficient memory\n");
		//exit(EXIT_FAILURE);
	    }

	   r = readlink(fname, linkname, sp->st_size + 1);

	   if (r < 0) 
            {
		perror("lstat");
		//exit(EXIT_FAILURE);
	    }

	   if (r > sp->st_size) 
           {
		fprintf(stderr, "symlink increased in size "
		                "between lstat() and readlink()\n");
		//exit(EXIT_FAILURE);
	    }

	   linkname[sp->st_size] = '\0';

	   printf("->%s",linkname);

	   //exit(EXIT_SUCCESS);
  }
  printf("</p>");

}

int ls_dir(char *dname)
{
  DIR *dir;
  struct dirent *dp;

  if ( (dir = opendir (dname)) == NULL){
     printf("<p>can't open %s</p>", dname); 
    
  }

  while ((dp = readdir (dir)) != NULL) 
  {
	ls_file(dp->d_name);
  }

  close(dir);

}

ls(int argc, char *argv[])
{
  struct stat mystat, *sp;
  int r;
  char *s;
  char name[1024], cwd[1024];

  s = argv[1];
  if (argc==1)
     s = "./";

  sp = &mystat;
  if (r = lstat(s, sp) < 0){
     printf("<p>no such file %s</p>", s); //exit(1);
  }
  strcpy(name, s);
  if (s[0] != '/'){    // name is relative : get CWD path
     getcwd(cwd, 1024);
     strcpy(name, cwd); strcat(name, "/"); strcat(name,s);
  }
  if (S_ISDIR(sp->st_mode))
  {
      ls_dir(name);
  }
  else
  {
      
      ls_file(name);
  }
}

cat(/*int argc, */ENTRY *argv)
{
	
	FILE *fp; 
	int c;
	if (strcmp(argv[1].value,"")==0)//(strcmp(argv,"\0")==0 | strcmp(argv,"")==0) 
	{
		printf("<p>Missinig parameters</p>");
		return;
	}

	fp = fopen(argv[1].value, "r");

	if (fp==NULL) 
	{
		printf("<p>Bad input</p>");
		//fclose(fp);
		return;
	}

	printf("<textarea>");

	while ((c = fgetc(fp)) != EOF)
	{
		putchar(c);  
	}
	
	fclose(fp);
	
	printf("</textarea>");
	printf("<p/>");
}
                                    
//cp(/*int argc, */ENTRY *argv)  
/*{
	int n, total=0;
	if (strcmp(argv[2].value,"")==0 | strcmp(argv[1].value,"")==0)
	{
		printf("<p>Missinig parameters</p>");
		return;
	}
	fp = fopen(argv[1].value, "r");
	if (fp == NULL) 
	{
		printf("<p>fp == NULL </p>");
		return;
	} 
	gp = fopen(argv[2].value, "w+");
	if (gp == NULL)
	{
		printf("<p>gp == NULL </p>");
		return;
	}
 
	while (n=fread(buf,1,BLKSIZE,fp))
	{
		fwrite(buf, 1, n, gp);
		total += n;
	}
	printf("total = %d\n", total);
	fclose(fp); 
	fclose(gp);
}     */             


              
                                     
cp(ENTRY *argv)         
 {                                 
   int n, total=0;                   
   if (strcmp(argv[2].value,"")==0 | strcmp(argv[1].value,"")==0)
   {
	printf("<p>Missinig parameters</p>");
	return;
   }           
                                     
   fd = open(argv[1].value, O_RDONLY);     

   if (fd < 0)
   {
	printf("<p>fp == NULL </p>");
	//return;
   }              

   gd=open(argv[2].value,O_WRONLY|O_CREAT);

   if (gd < 0)
   {
	printf("<p>gp == NULL </p>");
	//return;
   }      
                                     
   while (n=read(fd, buf, BLKSIZE))  
   {                                
      write(gd, buf, n);            
      total += n;                    
   }                             
   printf("<p>total=%d</p>",total);       
                                     
   close(fd); 
   close(gd);             
 }       

                               
main(int argc, char *argv[]) 
{
  int i, m, r;
  char cwd[128];
  
  m = getinputs();    // get user inputs name=value into entry[ ]
  getcwd(cwd, 128);   // get CWD pathname

  printf("Content-type: text/html\n\n");
  printf("<p>pid=%d uid=%d cwd=%s\n", getpid(), getuid(), cwd);

  printf("<H1>Echo Your Inputs</H1>");
  printf("You submitted the following name/value pairs:<p>");
 
  for(i=0; i <= m; i++)
     printf("%s = %s<p>", entry[i].name, entry[i].value);
  printf("<p>");


  /*****************************************************************
   Write YOUR C code here to processs the command
         mkdir dirname
         rmdir dirname
         rm    filename
         cat   filename
         cp    file1 file2
         ls    [dirname] <== ls CWD if no dirname
  *****************************************************************/

        int ID = findCmd(entry[0].value);
                switch(ID){
                case 0 : cp(/*argc,*/entry);     break;
                case 1 : 
			if(strcmp(entry[1].value, "")==0)
			{
				printf("<p>Missing dir name</p>");
			}
			else if (mkdir(entry[1].value, 0777) < 0)
			{
		         	printf("errno=%d : %s\n", errno, strerror(errno));
		      	}
		    
			break;
                case 2 : 
			if(strcmp(entry[1].value, "")==0)
			{
				printf("<p>Missing dir name</p>");
			}
			else if (rmdir(entry[1].value, 0777) < 0)
			{
		         	printf("errno=%d : %s\n", errno, strerror(errno));
		      	}
		    
			break;
                case 3 : ls(argc,argv);       break;
                //case 4 : cd();       break;
                case 5 : cat(/*getinputs(), */entry);    break;
                //case 6 : pwd();      break;
                case 7 : 
			if(strcmp(entry[1].value, "")==0)
			{
				printf("<p>Missing dir name</p>");
			}
			else if (remove(entry[1].value) < 0)
			{
		         	printf("errno=%d : %s\n", errno, strerror(errno));
		      	}
		    
			break;
                //case 8 : quit();     break;
                //case 9 : help();     break;

                //case 11: reload();	 break;
                //case 12: save();     break;

                default:
		        printf("<p>bad command/input</p>");
		        break;
              }

 
  // create a FORM webpage for user to submit again 
  printf("</title>");
  printf("</head>");
  printf("<body bgcolor=\"#FF0000\" link=\"#330033\" leftmargin=8 topmargin=8");
  printf("<p>------------------ DO IT AGAIN ----------------\n");
  
  //printf("<FORM METHOD=\"POST\" ACTION=\"http://cs560.eecs.wsu.edu/~kcw/cgi-bin/mycgi\">");
  printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~zhang/cgi-bin/mycgi\">");
  //------ NOTE : CHANGE ACTION to YOUR login name ----------------------------
  //printf("<FORM METHOD=\"POST\" ACTION=\"http://cs560.eecs.wsu.edu/~YOURNAME/cgi-bin/mycgi\">");
  
  printf("Enter command : <INPUT NAME=\"command\"> <P>");
  printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>");
  printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>");
  printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>");
  printf("</form>");
  printf("------------------------------------------------<p>");

  printf("</body>");
  printf("</html>");
  
}
