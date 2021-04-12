#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10

typedef struct node{
  struct node *children;
  struct node *next;
  struct node *pre;
  struct node *parent;
  int type; //1 = dir, 0 = file
  char name[64];
  char *self_loc[64];
}NODE;

FILE* file;

NODE *root, *cwd, *pathNames;                 /* root and CWD pointers */
char line[128];                               /* user input line */
char command[16], pathname[64];               /* user inputs */
char dirname[64], basename[64];               /* string holders */

int jump;

char *cmd[] = {"menu", "mkdir", "rmdir", "ls", "cd", "creat", "pwd", "rm",
               "quit", "help", "?",  "reload", "save", 0};

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

NODE *initialize(NODE *tmp, char *_name, NODE *nex, NODE *prv, NODE *child, NODE *par, int _type)
{
    tmp = malloc(sizeof(NODE));
    tmp->next = nex;
    tmp->pre = prv;
    tmp->children = child;
    tmp->parent = par;
    strcpy(tmp->name,_name);
    tmp->type = _type;

    strcpy(tmp->self_loc, "\0");

    return tmp;
}


main()
{
        root = initialize(root, "/", NULL, NULL, NULL, NULL, 1);      /* initialize / node of the file system tree */
        strcpy(root->self_loc,"/");
        cwd = root;

    while(1)
    {
        pathNames = initialize(root, "", NULL, NULL, NULL, NULL, 0);

        printf("input a commad : ");

        fgets(line, 128, stdin); // input at most 128 chars BUT has \r at end
        line[strlen(line)] = '\0';          // kill the \r at end.

        sscanf(line, "%s %s", command, pathname);

        //printf("command: %s\n", command);
        //printf("pathname: %s\n", pathname);

        int ID = findCmd(command);
                switch(ID){
                case 0 : menu();     break;
                case 1 : mkdir();    break;
                case 2 : rmdir();    break;
                case 3 : ls();       break;
                case 4 : cd();       break;
                case 5 : creat();    break;
                case 6 : pwd();      break;
                case 7 : rm();	     break;
                case 8 : quit();     break;
                case 9 : help();     break;

                case 11: reload();	 break;
                case 12: save();     break;

                default:
                printf("bad command/input\n");
                break;
              }

    strcpy(command,"\0");
    strcpy(pathname,"\0");
    strcpy(line,"\0");
    strcpy(dirname,"\0");
    strcpy(basename,"\0");
    }
}

reload()
{

    if(strcmp(pathname, "") == 0)
      {
        printf("   ERROR: no file name specified.\n");
        return 0;
      }
      printf("reloading %s...\n", pathname);
      file = fopen(pathname, "r");           // open a FILE stream for READ

      if(file == NULL)
      {
        printf("   ERROR: couldn't open file.\n");
        return 0;
      }
      root = initialize(root, "/", NULL, NULL, NULL, NULL, 1);      /* initialize / node of the file system tree */
      strcpy(root->self_loc,"/");
      cwd = root;
      char type, line[100];
      // at end of file
      while(1)
      {
        // get line of file
        fgets(line, 100, file);
        // Did we read the end of the file?
        if(feof(file)) break;
        //printf("line: %s\n", line);
        line[strlen(line)] = 0; line[strlen(line)-1] = 0;
        if(line != NULL || strcmp(line, "") != 0)
        {

          // split line
          char *splitter = strtok(line, "\t");
          type = *splitter;
          splitter = strtok(NULL, "\t");
          strcpy(pathname, splitter);
          //printf("pathname: %s\n", pathname);
          switch(type)
          {
            case 'F':
              creat();
            break;
            case 'D':
              mkdir();
            break;
          }
        }

      }

      fclose(file);

}


int save()
{
  if(strcmp(pathname, "") == 0)
  {
    printf("   ERROR: no file name specified.\n");
    return 0;
  }

  printf("saving %s...\n", pathname);

  file = fopen(pathname, "w+");

  if(file == NULL)
  {
    printf("   ERROR: couldn't open file.\n");
    return 0;
  }

  pathNames = initialize(pathNames, "", NULL, NULL, NULL, NULL, 1);
  save_helper(root);
  pathNames = pathNames->next;
  while(pathNames!=NULL)
  {
      if(pathNames->type)
      {
          fprintf(file, "D\t%s\n", pathNames->self_loc);
      }
      else
      {
          fprintf(file, "F\t%s\n", pathNames->self_loc);
      }
      pathNames = pathNames->next;
  }

  fclose(file);

}

save_helper(NODE *ptr)
{
     while(ptr != NULL)
     {
         if(ptr->children != NULL)
         {
             NODE *tmp = ptr->children;
             while(ptr->children->next != NULL)
             {
                 ptr->children = ptr->children->next;
             }

             save_helper(ptr->children);
             ptr->children = tmp;
         }


        strcpy(pathNames->self_loc, ptr->self_loc);
        pathNames->type = ptr->type;
        NODE *temp = pathNames;
        pathNames = pathNames->pre;
        if(pathNames == NULL)
        {
            pathNames = initialize(pathNames, "", NULL, NULL, NULL, NULL, 1);
        }
        pathNames->next = temp;


         NODE *tmp_par = ptr;
         ptr = ptr->pre;

    }

}

quit()
{
    exit(0);
}

help()
{
    menu();
}

massFrees(NODE* ptr, NODE *tmp)
{
       //printf("At %s\n", ptr->self_loc);
    while(ptr != NULL)
    {
        if(ptr->children != NULL)
        {
            //printf("deleting children of %s, the children->next is: %s\n",ptr->self_loc, ptr->children->next->self_loc);
            massFrees(ptr->children,tmp);
        }

        NODE *tmp_par = ptr;
        ptr = ptr->next;

        if(tmp == tmp_par)
        {
            jump = 1;

        }

        free(tmp_par);

        //    printf("clearing parent %s\n", tmp_par->self_loc);

    }
}

massFree(NODE *ptr, NODE *tmp)
{
    ptr->pre = NULL;
    ptr->next = NULL;
    ptr->parent = NULL;
    massFrees(ptr,tmp);
}


ImakeIt(int dir)
{

    breakThePath(pathname, 1);
    if(basename[0] != '\0')
    {
        NODE *tmp_cwd = cwd;
        if(cds())
        {
            NODE *temp = initialize(temp, basename, NULL, NULL, NULL, cwd, dir);
            if(cwd != root)
            { strcpy(temp->self_loc,cwd->self_loc); }
            strcat(temp->self_loc, "/");
            strcat(temp->self_loc, temp->name);

            if(cwd->children == NULL)
            {
                cwd->children = temp;
                //temp->parent = cwd;
            }
            else
            {
                NODE *child = cwd->children;
                int exist = 1; //check existance
                while(child->next!=NULL)
                {
                    if(strcmp(child->name,basename) == 0 & child->type == dir)
                    {
                        exist = 0;
                        break;
                    }
                    child = child->next;
                }

                if(strcmp(child->name,basename) == 0 & child->type == dir)
                {
                    exist = 0;
                }
                if(exist)
                {
                    child->next = temp;
                    temp->pre = child;
                }
            }
        }

        cwd = tmp_cwd;
    }

}

IdestIt(int dir)
{
    breakThePath(pathname, 1);
    NODE *tmp_cwd = cwd;
    if(cds())
    {

        if(cwd->children != NULL)
        {
            NODE *child = cwd->children;
            while(child!=NULL)
            {

                if(strcmp(child->name,basename)==0 & child->type == dir)
                {
                    //printf("basename : %s\n", basename);
                    //printf("cwd->name : %s\n", cwd->name);

                    NODE *temp = child->next;

                    child = child->pre;
                    if(child != NULL)
                    {
                        massFree(child->next, tmp_cwd);
                        child->next = temp;
                        if(temp != NULL)
                        {

                            temp->pre = child;
                        }
                    }
                    else
                    {
                        massFree(cwd->children, tmp_cwd);
                        cwd->children = temp;

                    }


                    break;
                }

                child = child->next;
            }
        }
    }

    if(jump)
    {
        cwd = root;
    }
    else
    {
        cwd = tmp_cwd;
    }

}


creat()
{
    ImakeIt(0);
}

rm()
{
    IdestIt(0);
}


rmdir()
{
    jump = 0;
    IdestIt(1);
    jump = 0;
}

mkdir()
{
    ImakeIt(1);
}

ls()
{
    if(cwd->children != NULL)
    {
        NODE *tmp = cwd->children;
        while(tmp->next!=NULL)
        {

            printf("%s\n",tmp->name);
            tmp = tmp->next;
        }

        printf("%s\n",tmp->name);
    }
    else
    {
        printf("\n");
    }
}

cd()
{
    breakThePath(pathname, 0);
    if(cd_helper() == 0)
    {
        printf("Dir not found\n");
        return 0;
    }
    return 1;

}

cds()
{
    if(cd_helper() == 0)
    {
        printf("Dir not found\n");
        return 0;
    }
    return 1;

}

int cd_helper()
{
    if(pathNames != NULL)
    {
        if(strcmp(pathname,"-") == 0)
        {
            if(cwd != root)
            {
                cwd = cwd->parent;
                pathNames = pathNames->next;
                if(cd_helper())
                { return 1;}
            }
            return 0;
        }
        else if(strcmp(pathNames->name,"/") == 0)
        {
            cwd = root;
            pathNames = pathNames->next;
            if(cd_helper())
            { return 1;}
            return 0;
        }
        else
        {

            if(cwd->children !=NULL)
            {
                NODE *child = cwd->children;

                while(child->next != NULL)
                {
                    if(strcmp(child->name, pathNames->name) == 0 & child->type == 1)
                    {

                        cwd = child;
                        pathNames = pathNames->next;

                        if(cd_helper())
                        {
                            return 1;
                        }

                        return 0;

                    }

                    child = child->next;

                }

                if(strcmp(child->name, pathNames->name) == 0 & child->type == 1)
                {
                    cwd = child;
                    pathNames = pathNames->next;

                    if(cd_helper())
                    {
                        return 1;
                    }

                    return 0;

                }

                return 0;

            }

            return 0;
        }
    }

    return 1;
}

breakThePath(char *name, int mkOrNot)
{
    NODE *tmp= initialize(tmp, "", NULL, NULL, NULL, NULL, 1);
    int abs = 0;
    if(name[0] == '/')
    {
        abs = 1;
    }

    int i;

    for(i = abs; name[i] != '\0'; i++)
    {
        if(name[i] != '/')
        {
            sprintf(tmp->name,"%s%c",tmp->name,name[i]);
        }
        else
        {
            tmp->next = initialize(tmp->next, "", NULL, tmp, NULL, NULL, 1);
            tmp = tmp->next;
        }

    }


    if(i == 1 && abs)
    {
        strcpy(tmp->name,"/");
    }

    else if(mkOrNot)
    {
        strcpy(basename, tmp->name);
        tmp = tmp->pre;
        //else...
        if(tmp != NULL)
        {
            tmp->next = NULL;
            while(tmp->pre != NULL)
            {
                tmp = tmp->pre;
            }

            if(abs)
            {
                NODE *temp = initialize(temp, "/", tmp, NULL, NULL, NULL, 1);
                tmp->pre = temp;
                tmp = temp;
            }
        }
        else if(abs)
        {
            tmp = initialize(tmp, "/", NULL, NULL, NULL, NULL, 1);
        }
    }

    else
    {

            while(tmp->pre != NULL)
            {
                tmp = tmp->pre;
            }

            if(abs)
            {
                NODE *temp = initialize(temp, "/", tmp, NULL, NULL, NULL, 1);
                tmp->pre = temp;
                tmp = temp;
            }

    }


    pathNames = tmp;

}

pwd()
{
    if(cwd == NULL)
    {
        printf("cwd is NULL\n");
    }
    else
    {
        printf("%s\n", cwd->self_loc);
    }
}
menu()
{
    printf("********************\n");
    printf("\n");

    printf("menu             : print this menu\n");
    printf("mkdir  pathname  : make a new directiry for the pathname\nrmdir  pathname  : rm the directory, if it is empty.\ncd    [pathname] : change CWD to pathname, or to / if no pathname.\nls    [pathname] : list the directory contents of pathname or CWD\npwd              : print the (absolute) pathname of CWD\ncreat  pathname  : create a FILE node.\nrm     pathname  : rm a FILE node.save   filename  : save the current file system tree in a file\nreload filename  : re-initalize the file system tree from a file\nquit             : save the file system tree, then terminate the program.\n");

    printf("\n");
    printf("********************\n");
}
