#include "functions/funcs.h"

main(int argc, char *argv[ ])
{
  int i,cmd; 
  char line[128], cname[64];

  if (argc>1){
    if (strcmp(argv[1], "-d")==0)
        DEBUG = 1;
  }

  init();

  while(1){
      printf("P%d running: ", running->pid);

      /* zero out pathname, parameter */
      for (i=0; i<64; i++){
          pathname[i]=parameter[i] = 0;
      }      
      /* these do the same; set the strings to 0 */
      memset(pathname, 0, 64);
      memset(parameter,0, 64);

      printf("input command : ");
      gets(line);
      if (line[0]==0) continue;

      sscanf(line, "%s %s %64c", cname, pathname, parameter);

      cmd = findCmd(cname);
      switch(cmd){

           case 0 : make_dir();               break;
           case 1 : cd();             break;
           case 2 : pwd();       break;
           case 3 : ls();              break;
           case 4 : mount();                  break;
           case 5 : umount(pathname);         break;
           case 6 : creat_file();             break;
           case 7 : rm_dir();                  break;
           case 8 : rm_file();                break;

           case 9 : open_file();              break;
           case 10: close_file();             break;

           case 11: read_file();              break;
           case 12: write_file();             break;
           case 13: cat_file();               break;

           case 14: cp_file();                break;
           case 15: mv_file();                break;

           case 16: pfd();                    break;
           case 17: lseek_file();             break;
 /*          case 18: rewind_file();            break;
           case 19: mystat();                 break;

           case 20: pm();                     break;
*/
           case 21: menu();                   break;

           //case 22: access_file();            break;
           case 23: chmod_file();             break;
           case 24: chown_file();             break;

           //case 25: cs();                     break;
           //case 26: do_fork();                break;
           //case 27: do_ps();                  break;
           //case 28: do_kill();                break;

           case 29: quit();                   break; 
           case 30: do_touch();               break;

           //case 31: sync();                   break;
           case 32: my_link(); break;
           case 33: my_unlink(); break;
           case 34: my_symlink(); break;
           case 35: chgrp_file(); break;
           case 36: my_stat(); break;

           default: printf("invalid command\n");
                    break;
      }
  }
} /* end main */

// NOTE: you MUST use a function pointer table





