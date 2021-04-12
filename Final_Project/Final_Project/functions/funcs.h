#include "functions/etc.h"


mount()
{
    char line1[128],line[128], buf[BLKSIZE];
    strcpy(line, pathname);
    char *b = basename(line);
    strcpy(line1, parameter);
    char *mb = basename(line1);

      char cwd[128];
      getcwd(cwd, 128);
      int x = strlen(cwd);
      int i;

      int j = strlen(pathname);
      if(j>0)
      {
          cwd[x]='/';
          x++;
          for(i=x; i < 1+x+j; i++)
          {
              cwd[i] = pathname[i-x];
          }
      }
      else
      {
            printf("%s\n", cwd);
      }


      MOUNT *mp;

      int ino = getino(running->cwd->dev, parameter, 1);

      if(ino < 0)//root??
      {
          printf("Dst dir cannot be found!\n");
          return -1;
      }

      int dev = open(cwd, O_RDWR);
      //printf("dev= %d\n", dev);

      if (dev < 0){
         printf("panic : can't open root device\n");
         return-2;
      }

      //printf("The name is '%s'\n", b);
      mp = getmountp(b);

      if(mp==NULL)
      {
          return -3;
      }

      MINODE *mnt = iget(running->cwd->dev, ino);          /* get root inode */


      if(mnt->refCount==1)
      {
          strcpy(mp->name, b);
          pwd_helper(getUpperRelatedParentOf(mnt), mnt, mb);
          strcpy(mp->mount_name, mb);

          mp->dev = dev;
          mp->busy = BUSY;
          MINODE *tmp = iget(dev, 2);
          mp->mounted_inode = tmp;
          mnt->mountptr = mp;
          //mnt->ino=2;
          //mnt->dev=dev;
/*
          char buff[BLKSIZE];
          GD *gps;
          get_block(dev, 2, &buff);
          gps = (GD *)buff;

          int block = (2-1) / INODES_PER_BLOCK + gps->bg_inode_table;
          int pos = (2-1) % INODES_PER_BLOCK;
          get_block(dev, block, &buff);
          memcpy(&mnt->INODE, (((INODE*)buff) + pos),sizeof(INODE));

          int i;
          for(i = 0; i<NMINODES; i++)
          {
              printf("minode[%d]->dev = %d, minode[%d]->ino = %d\n", i, minode[i].dev, i, minode[i].ino);
          }*/

          //char buff[BLKSIZE];
          //MINODE *tmp = iget(dev, 2);
          //get_block(dev, tmp->INODE.i_block[0], &buff);
          //mnt->INODE.i_block[0]=buff;
          //memcpy(&mnt->INODE, &mp->mounted_inode, sizeof(MINODE));

          mnt = mp->mounted_inode;
          //mnt->dirty=1;

          return 0;
    }
      else
      {
          printf("The dst file is being used by %d processes\nCannot be Mounted!!\n", mnt->refCount);
          return -4;
      }
}

umount(char *path)
{

}

mv_file()
{
    if(search(running->cwd->dev, pathname, 0) > 0)// && search(running->cwd->dev, parameter, 0) <=0)
    {
        int dev = running->cwd->dev;
        MINODE *src = iget(dev, getino(dev, pathname, 0));
        char *b, line[128];
        strcpy(line, pathname);
        b = basename(line);
        //my_link();
        cp_file();
        src->refCount++;
        rm_child(getUpperRelatedParentOf(src), basename(pathname), 0);

        /*
         *
        1. verify src exists; get its INODE in ==> you already know its dev
        2. check whether src is on the same dev as src

                      CASE 1: same dev:
        3. Hard link dst with src (i.e. same INODE number)
        4. unlink src (i.e. rm src name from its parent directory and reduce INODE's
                       link count by 1).

                      CASE 2: not the same dev:
        3. cp src to dst
        4. unlink src*/
    }
    else
    {
        printf("Source does not exist\n");// or Dst exists already!!\n");
    }
}

cp_file()
{

    int fd = myopen(pathname, 0);
    //printf("fd = %d\n", fd);
    if(fd>=0)
    {
        int dev = running->cwd->dev;
        int ino = ialloc(dev);
        int blk = balloc(dev);
        if(mymk(0, parameter, ino, blk, 0) >=0)
        {

            int gd = myopen(parameter, 1);
            //printf("gd = %d\n", gd);
            if(gd >=0)
            {

                int n;
                char buf[BLKSIZE];

                while(n = myread(fd, &buf, BLKSIZE, 2) ){

                       mywrite(gd, buf, n, 0);  // notice the n in write()
                }

                close_file_helper(gd);
            }
            else
            {
                //it should not occur...
                rm_helper(0);
                printf("Well, some problem occur in dst\n");
            }

        }
        else
        {
            printf("Failed to copy des file!\n");
        }
        close_file_helper(fd);
    }
    else
    {
        printf("Error occur because of the source file\n");
    }

}

int myopen(char *path, int mode)
{

    MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, path, 0));
    mip->refCount--;

    if(S_ISLNK(mip->INODE.i_mode))
    {
            char buf[60];
            read_link((char *)mip->INODE.i_block, &buf); //buff = path in symlink's blok

            //printf("It was %s points to %s\n", path, buf);
            int gd = myopen(buf, mode);

            return gd;
    }
    else
    {
        if(mip!=NULL)
        {

            int i = myopen_file(path, mode);

            return i;
        }
        else
        {
            printf("Open failed!\n");

            return -1;
        }
    }


}


int myopen_file(char *path, int mode)
{
    //printf("opening %s\n", path);
    MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, path, 0));

    OFT *oftp = 0;

    int fd = chkFd(mip);

    //if opened for editing
    if(fd == -2)
    {
        mip->refCount--;
        return -2;
    }

    //if not opened (for R) yet
    if(fd == -1)
    {
        fd = falloc();
    }


    if(fd > -1)
    {
            //printf("mode = %d\n", mode);
            if(mode == 0 | mode == 1 | mode == 2 | mode == 3)
            {
                oftp = malloc(sizeof(OFT));

                switch(mode){
                   case 0 : oftp->offset = 0;     // R: offset = 0
                            mip->INODE.i_atime = do_touch();
                            break;
                   case 1 : truncates(mip);        // W: truncate file to 0 size
                            oftp->offset = 0;
                            mip->INODE.i_atime = mip->INODE.i_mtime = do_touch();
                            //printf("1.refcount = %d\n", mip->refCount);
                            break;
                   case 2 : oftp->offset = 0;     // RW: do NOT truncate file
                            mip->INODE.i_atime = mip->INODE.i_mtime = do_touch();
                            break;
                   case 3 : oftp->offset =  mip->INODE.i_size;  // APPEND mode
                            mip->INODE.i_atime = mip->INODE.i_mtime = do_touch();
                            break;
                }

                oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND
                oftp->refCount = 1;
                oftp->inodeptr = mip;  // point at the file's minode[]

                running->fd[fd]=oftp;

                oftp->inodeptr->dirty=1;
                oftp->inodeptr->refCount++;

                iput(oftp->inodeptr);

                return fd;
            }
            else
            {
                printf("invalid mode\n");
                mip->refCount--;
                return(-1);
            }

    }
    else
    {
        mip->refCount--;
        printf("Already running/opening too many, or file had been editing\n");
        return -1;
    }
}


int write_file()
{
    int fd = atoi(pathname);

    OFT *ofpt = running->fd[fd];

    //printf("Before write: fd->ino->refCount = %d\n", ofpt->refCount);

    if(S_ISLNK(ofpt->inodeptr->INODE.i_mode))
    {


            //char buf[60], buff[strlen(parameter)];
            //char *f = fd+'0';
            //char b[23]="/lost+found/reading/fdx";
            //b[22]=f;
            //char c[3]="fdx";

            char par[128], buf[60];

            strcpy(par, parameter);

            //read_link(b, &buf);//buf = path to symlink itself
            //printf("path to symlink itself = %s\npath in symlink's block = %s\n", buf, buff);

            read_link((char *)ofpt->inodeptr->INODE.i_block, &buf); //buff = path in symlink's blok

            //printf("the linked path is '%s'\n", buff);

            strcpy(pathname, buf);
            if(ofpt->mode==1)
            {
                strcpy(parameter, "W");
            }
            else if(ofpt->mode==3)
            {
                strcpy(parameter, "APPEND");
            }
            //printf("pathname = %s\n", pathname);

            int gd = open_file_helper();

            //printf("gd = %d\n", gd);
            //myread(gd, &buff, nbytes, 1);

            char *g = gd+'0';
            //c[2]=g;

            //printf("gd = %c\n", g);
            //printf("Par = %s\n", par);

            pathname[0]=g;
            pathname[1]=0;
            strcpy(parameter, par);

            int z = write_file();

            close_file_helper(gd);
            printf("\n");

            return z;
    }
    else
    {
        if(ofpt != 0 && (ofpt->mode == 1 | ofpt->mode==3))
        {
            char buff[BLKSIZE];

            memset(buff, 0, BLKSIZE);
            printf("input the string :");
            scanf("%s", &buff);
            //printf("Inputed %s\n", buff);
            int nbytes=atoi(parameter);
            //printf("nbytes = %d\n", nbytes);
            int i = mywrite(fd, buff, nbytes, 1);

            //printf("%s\n", buff);
            //printf("After write: fd->ino->refCount = %d\n", ofpt->refCount);
            return i;
        }
        else
        {
            printf("File is not opened for W or APPEND, or bad input. Open failed!\n");

            return -1;
        }
    }


}

int mywrite(int fd, char *buf, int nbytes, int prt)
{
    OFT *oftp = running->fd[fd];

    int count = 0;
//    int avil = oftp->inodeptr->INODE.i_size - oftp->offset;
    //printf("avaible is %d\tsize is %d\toffset is %d\n", avil,oftp->inodeptr->INODE.i_size, oftp->offset);
    int lbk, startByte, blk, remain;

    char wbuf[BLKSIZE];
    memset(wbuf, 0, BLKSIZE);
    char *cq = buf;                // cq points at buf[ ]

    //printf("nbytes of write=%d\nbuf of write=%s\b", nbytes, buf);

    //printf("\n*****************************************************************\n");

    while (nbytes > 0 ){

       //Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

             lbk       = oftp->offset / BLKSIZE;
             startByte = oftp->offset % BLKSIZE;

             //printf("nbytes=%d\n", nbytes);

             //printf("init lbk = %d\n", lbk);
       // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT

       //printf("block number is %d\n", lbk);

       if (lbk < 12){                     // lbk is a direct block
           //printf("direct block\n");
           if (oftp->inodeptr->INODE.i_block[lbk] == 0){   // if no data block yet


              oftp->inodeptr->INODE.i_block[lbk] = balloc(oftp->inodeptr->dev);// MUST ALLOCATE a block

              // write a block of 0's to blk on disk: OPTIONAL for data block
              //                                      but MUST for I or D blocks
           }

           blk = oftp->inodeptr->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk

       }
       else if (lbk >= 12 && lbk < 256 + 12) {
            //printf("indirect blocks\n");

           if (oftp->inodeptr->INODE.i_block[12] == 0){   // if no data block yet

              oftp->inodeptr->INODE.i_block[12] = balloc(oftp->inodeptr->dev);// MUST ALLOCATE a block

              // write a block of 0's to blk on disk: OPTIONAL for data block
              //                                      but MUST for I or D blocks
           }

           get_block(running->cwd->dev, oftp->inodeptr->INODE.i_block[12], &wbuf);

           blk = *((long *)wbuf+(lbk - 12));

       }
       //else if (lbk >= 12+256 && lbk < 256*256 + 12) {
       else {
            printf("double indirect blocks\n");

           if (oftp->inodeptr->INODE.i_block[13] == 0){   // if no data block yet

              oftp->inodeptr->INODE.i_block[13] = balloc(oftp->inodeptr->dev);// MUST ALLOCATE a block

              // write a block of 0's to blk on disk: OPTIONAL for data block
              //                                      but MUST for I or D blocks
           }

           get_block(running->cwd->dev, oftp->inodeptr->INODE.i_block[13], &wbuf);

           blk = *((long *)wbuf+((lbk-256-12)/256));

           if(blk == 0)
           {
               //printf("Double link show to be 0\n");
               blk = balloc(oftp->inodeptr->dev);
               put_block(running->cwd->dev, oftp->inodeptr->INODE.i_block[13], (char *)blk);
           }


           get_block(running->cwd->dev, blk, &wbuf);

           blk = *((long *)wbuf+((lbk-256-12)%256));
       }
/*
       else
       {
           //triple link, just for fun
           get_block(running->cwd->dev, oftp->inodeptr->INODE.i_block[14], &readbuf);

           blk = (reafbuf / BLKSIZE)  + (((lbk - 12)/256)/256);

           get_block(running->cwd->dev, blk, &readbuf);

           blk = (readbuf / BLKSIZE) + (((lbk - 12)/256)%256);

           get_block(running->cwd->dev, blk, &readbuf);
           blk = (readbuf / BLKSIZE) + ((lbk - 12)%(256*256));
       }
*/
       //printf("\tblock is %d\n", blk);
       get_block(oftp->inodeptr->dev, blk, &wbuf);   // read disk block into wbuf[ ]
       char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
       remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

       while (remain > 0){               // write as much as remain allows

             *cp++ = *cq++;              // cq points at buf[ ]
             count++;
             nbytes--; remain--;         // dec counts
             oftp->offset++;             // advance offset
             if (oftp->offset > oftp->inodeptr->INODE.i_size)  // especially for RW|APPEND mode
                 oftp->inodeptr->INODE.i_size++;    // inc file size (if offset > fileSize)
             if (nbytes <= 0) break;     // if already nbytes, break

       }
       put_block(oftp->inodeptr->dev, blk, &wbuf);   // write wbuf[ ] to disk

       // loop back to while to write more .... until nbytes are written
    }

    oftp->inodeptr->dirty = 1;       // mark mip dirty for iput()
    //iput(oftp->inodeptr);
    if(prt==1)
    {
        printf("wrote %d char into file descriptor fd=%d\n", count, fd);
    }
    //return nbytes;
    return count;
}


int read_file()
{

    int fd = atoi(pathname);
    int nbytes = atoi(parameter);

    OFT *ofpt = running->fd[fd];

    if(S_ISLNK(ofpt->inodeptr->INODE.i_mode))
    {

            //char buf[60], buff[nbytes];
            //char *f = fd+'0';
            //char b[23]="/lost+found/reading/fdx";
            //b[22]=f;
            //char c[3]="fdx";

            char par[128], buf[60];

            strcpy(par, parameter);

            //read_link(b, &buf);//buf = path to symlink itself
            //printf("path to symlink itself = %s\npath in symlink's block = %s\n", buf, buff);

            read_link((char *)ofpt->inodeptr->INODE.i_block, buf); //buff = path in symlink's blok

            //printf("the linked path is '%s'\n", buff);

            strcpy(pathname, buf);
            if(ofpt->mode==0)
            {
                strcpy(parameter, "R");
            }
            else if(ofpt->mode==2)
            {
                strcpy(parameter, "RW");
            }
            //printf("pathname = %s\n", pathname);

            int gd = open_file_helper();

            //printf("gd = %d\n", gd);
            //myread(gd, &buff, nbytes, 1);

            char *g = gd+'0';
            //c[2]=g;

            //printf("gd = %c\n", g);
            //printf("Par = %s\n", par);

            pathname[0]=g;
            pathname[1]=0;
            strcpy(parameter, par);

            int z = read_file();

            close_file_helper(gd);
            printf("\n");

            return z;
    }
    else
    {
        if(ofpt != 0 && (ofpt->mode == 0 | ofpt->mode==2) && nbytes >=0)
        {
            char buf[nbytes];

            int i = myread(fd, &buf, nbytes, 1);

            //printf("%s\n", buf);

            return i;
        }
        else
        {
            printf("File is not opened for R or RW, or bad input. Open failed!\n");

            return -1;
        }
    }
}


int myread(int fd, char *buf, int nbytes, int prt)
{
    OFT *oftp = running->fd[fd];

    int count = 0;

    int avil = oftp->inodeptr->INODE.i_size - oftp->offset;

    //printf("avaible is %d\tsize is %d\toffset is %d\n", avil,oftp->inodeptr->INODE.i_size, oftp->offset);
    int lbk, startByte, blk, remain;

    char readbuf[BLKSIZE];
    char *cq = buf;                // cq points at buf[ ]

    //printf("\n*****************************************************************\n");

    while (nbytes && avil){

       //Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

             lbk       = oftp->offset / BLKSIZE;
             startByte = oftp->offset % BLKSIZE;

       // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT

       //printf("block number is %d\n", lbk);

       if (lbk < 12){                     // lbk is a direct block
           blk = oftp->inodeptr->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk

       }
       else if (lbk >= 12 && lbk < 256 + 12) {
            //  indirect blocks

           get_block(running->cwd->dev, oftp->inodeptr->INODE.i_block[12], &readbuf);

           //blk = ((long)readbuf / BLKSIZE) + (lbk-12);
           blk = *((long *)readbuf+(lbk-12));

       }
       //else if (lbk >= 12+256 && lbk < 256*256 + 12) {
       else {
            //  double indirect blocks
           get_block(running->cwd->dev, oftp->inodeptr->INODE.i_block[13], &readbuf);

           blk = *((long *)readbuf + ((lbk - 12)/256));

           get_block(running->cwd->dev, blk, &readbuf);

           //blk = ((long)readbuf / BLKSIZE) + ((lbk - 12)%256);
           blk = *((long *)readbuf+((lbk-256 - 12)%256));
       }
/*
       else
       {
           //triple link, just for fun
           get_block(running->cwd->dev, oftp->inodeptr->INODE.i_block[14], &readbuf);

           blk = (reafbuf / BLKSIZE)  + (((lbk - 12)/256)/256);

           get_block(running->cwd->dev, blk, &readbuf);

           blk = (readbuf / BLKSIZE) + (((lbk - 12)/256)%256);

           get_block(running->cwd->dev, blk, &readbuf);
           blk = (readbuf / BLKSIZE) + ((lbk - 12)%(256*256));
       }
*/
       //get the data block into readbuf[BLKSIZE]
       get_block(running->cwd->dev, blk, &readbuf);

       // copy from startByte to buf[ ], at most remain bytes in this block
       char *cp = readbuf + startByte;
       remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]
       while (remain > 0){

           if(prt == 1)
           {
            printf("%c", *cp);
           }
            *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
             oftp->offset++;           // advance offset
             count++;                  // inc count as number of bytes read
             avil--; nbytes--;  remain--;
             if (nbytes <= 0 || avil <= 0)
                 break;
       }

       // if one data block is not enough, loop back to OUTER while for more ...

   }

    if(prt < 1)
    {
        printf("\nmyread: read %d char from file descriptor %d\n", count, fd);
    }

    //printf("\ncount = %d\n", count);

    if(count == 0 & prt < 2)
    {
        printf("\n*****************************************************************\n");
    }

   return count;   // count is the actual number of bytes read

}

open_file()
{

    int fd = open_file_helper();

    if(fd > -1)
    {

        char *d = "/lost+found/reading";

        char *f = fd+'0';

        char b[23]="/lost+found/reading/fdx";
        b[22]=f;

        if(search(running->cwd->dev, d, 1) == -1)
        {
            mymk(1, d, ialloc(running->cwd->dev), balloc(running->cwd->dev), 0);
        }

        if(search(running->cwd->dev, b, -1)==-1)
        {
            symlinks(pathname, b);
        }
    }

    return fd;
}

//ofpt points to "real" file
//check is the "real" block had been cleaned when deleted the origin

int open_file_helper()
{
    char cmode[6];

    MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, pathname, 0));

    //printf("from open_file_helper: pathname=%s\n", pathname);


    OFT *oftp = 0;
    int mode;

    int fd = chkFd(mip);

    //if opened for editing
    if(fd == -2)
    {
        mip->refCount--;
        return -2;
    }

    //if not opened (for R) yet
    if(fd == -1)
    {
        fd = falloc();
    }

    memset(cmode, 0, sizeof(cmode));

    strncpy(cmode, parameter, sizeof(cmode));

    if(fd > -1)
    {
            mode = transMode(cmode);
            //printf("mode = %d\n", mode);
            if(mode == 0 | mode == 1 | mode == 2 | mode == 3)
            {
                oftp = malloc(sizeof(OFT));

                switch(mode){
                   case 0 : oftp->offset = 0;     // R: offset = 0
                            mip->INODE.i_atime = do_touch();
                            break;
                   case 1 : truncates(mip);        // W: truncate file to 0 size
                            oftp->offset = 0;
                            mip->INODE.i_atime = mip->INODE.i_mtime = do_touch();
                            //printf("1.refcount = %d\n", mip->refCount);
                            break;
                   case 2 : oftp->offset = 0;     // RW: do NOT truncate file
                            mip->INODE.i_atime = mip->INODE.i_mtime = do_touch();
                            break;
                   case 3 : oftp->offset =  mip->INODE.i_size;  // APPEND mode
                            mip->INODE.i_atime = mip->INODE.i_mtime = do_touch();
                            break;
                }

                oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND
                oftp->refCount = 1;
                oftp->inodeptr = mip;  // point at the file's minode[]

                running->fd[fd]=oftp;

                oftp->inodeptr->dirty=1;
                oftp->inodeptr->refCount++;

                //printf("from open: size = %d\n", oftp->inodeptr->INODE.i_size);
//printf("1.refcount = %d\n", mip->refCount);
                iput(oftp->inodeptr);

                return fd;
            }
            else
            {
                printf("invalid mode\n");
                mip->refCount--;
                return(-1);
            }

    }
    else
    {
        mip->refCount--;
        printf("Already running/opening too many, or file had been editing\n");
        return -1;
    }
}

read_link(char *path, char *buf)
{
    MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, path, 0));

    mip->refCount--;

    if(S_ISLNK(mip->INODE.i_mode))
    {
        //Correct? Need to Check
        char pwd[60];
        strcpy(pwd, (char *)mip->INODE.i_block);
        read_link(pwd, buf);
        return;
    }

    strcpy(buf, path);
}

close_file()
{
    int fd = atoi(pathname);
    int gd = close_file_helper(fd);

    if(gd != -1)
    {
        char *d = "/lost+found/reading";
        char *f = fd+'0';
        char b[3]="fdx";
        b[2]=f;

        MINODE *pip = iget(running->cwd->dev, getino(running->cwd->dev, d, 1));
        //pip->refCount++;
        rm_child(pip, b, 0);
        //printf("pip->refcount = %d, pip->ino=%d\n", pip->refCount, pip->ino);
    }



    //mymk(1, d, ialloc(running->cwd->dev));

    //symlinks(pathname, b);

}

int close_file_helper(int fd)
{
    OFT *oftp;

    if((fd >= 0 && fd < 10) && running->fd[fd] != 0)
    {

         oftp = running->fd[fd];
         //printf("Before close: fd->ino->refCount = %d\n", oftp->inodeptr->refCount);
         running->fd[fd] = 0;
         oftp->refCount--;
         if (oftp->refCount > 0) return 0;

         // last user of this OFT entry ==> dispose of the Minode[]
         iput(oftp->inodeptr);

         //printf("After close: fd->ino->refCount = %d\n", oftp->inodeptr->refCount);
         return 0;

    }
    else
    {
        printf("Failed closing file\n");
        return -1;
    }
}
lseek_file()
{
    int fd = atoi(pathname);
    int gd = atoi(parameter);
    lseek_file_helper(fd, gd);
}

int lseek_file_helper(int fd, int position)
{

    OFT *oftp;

    if((fd >= 0 && fd < 10) && running->fd[fd] != 0)
    {

         oftp = running->fd[fd];

         if(position >=0 && position < running->fd[fd]->inodeptr->INODE.i_size)
         {
             int originalPosition = oftp->offset;

             oftp->offset = position;

             return originalPosition;
         }

         return -1;

    }
    else
    {
        printf("Failed closing file\n");
        return -1;
    }
}


int pfd()
{
    int i;

    printf("Filename\tFD\tmode\toffset\n");
    printf("--------\t--\t----\t------\n");
    for(i = 0;i<10;i++)
    {
        if (running->fd[i]!= 0)
        {

            char *f = i+'0';
            char b[23]="/lost+found/reading/fdx";
            b[22]=f;

            MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, b, 0));
            mip->refCount--;

            printf("%s", (char *)mip->INODE.i_block);

            printf("\t\t%d\t",i);
            switch(running->fd[i]->mode)
            {
                case 0:
                    printf("READ\t");
                    break;
                case 1:
                    printf("WRITE\t");
                    break;
                case 2:
                    printf("R/W\t");
                    break;
                case 3:
                    printf("APPEND\t");
                    break;
                default:
                    printf("??????\t");//this should never happen
                    break;
            }
            printf("%li\n",running->fd[i]->offset);
        }
    }
    return 0;
}

dup(int fd)
{
    int i, find;

    for(i = 0; i<NFD; i++)
    {
        if(fd == running->fd[i])
        {
            find = 1;
            break;
        }
    }

    if(find==1)
    {
        running->fd[falloc()]=fd;
        running->fd[fd]->refCount++;
    }
    else
    {
        printf("Input is not an opened descriptor");
    }

}

dup2(int fd, int gd)
{
    if(fd != gd)
    {
        close_file(gd);
        running->fd[gd]=running->fd[fd];
    }
    else
    {
        printf("fd = %d is equal to gd = %d\n", fd, gd);
    }
}


do_touch()
{
    do_touch_helper(pathname);
}

do_touch_helper(char *path)
{
    //: modify the INODE's i_atime and i_mtime fields.
    MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, path, -1));
    mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L);
    mip->dirty=1;
    iput(mip);

}
chmod_file()
{
    char line[128];
    int mode=0;
    char tmp[64];

    MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, pathname, -1));

    strcpy(line, parameter);

    memset(tmp, 0, sizeof(tmp));

    strncpy(tmp, line, sizeof(tmp));

    mode = tmp[0]-48 << 6;
    mode |= tmp[1]-48 << 3;
    mode |= tmp[2]-48;

    mip->INODE.i_mode &= 0xFF000;
    mip->INODE.i_mode |= mode;

    mip->dirty=1;
    iput(mip);

    printf("ip->mode = %o\n", mip->INODE.i_mode);

}
chown_file()
{
    char line[128];

    char tmp[2];

    MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, pathname, -1));

    strcpy(line, parameter);

    memset(tmp, 0, sizeof(tmp));

    strncpy(tmp, line, sizeof(tmp));

    int num = atoi(tmp);

    if(num < NPROC)
    {
        mip->INODE.i_uid = num;
        mip->dirty=1;
        iput(mip);

        printf("ip->uid = %d\n", mip->INODE.i_uid);
    }
    else
    {
        printf("User doesn't exists!\n");
    }
}

chgrp_file()
{
    char line[128];

    char tmp[128];

    MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, pathname, -1));

    strcpy(line, parameter);

    memset(tmp, 0, sizeof(tmp));

    strncpy(tmp, line, sizeof(tmp));

    int num = atoi(tmp);


    //IDK How many groups we have

    //if(num < )
    //{
        mip->INODE.i_gid = num;
        mip->dirty=1;
        iput(mip);

        printf("ip->gid = %d\n", mip->INODE.i_gid);
    //}
    //else
    //{
        //printf("Group doesn't exists!\n");
    //}
}


my_link()
{
    if(parameter && pathname)
    {
        MINODE *pathToFile = iget(running->cwd->dev, getino(running->cwd->dev, pathname, 0));// a/b/c
        //it inc in mymk
        //pathToFile->refCount++;
        if(getino(running->cwd->dev, parameter, 0) == -1)
        {

            mymk(0, parameter, pathToFile->ino, pathToFile->INODE.i_block[0], 1);

            pathToFile->INODE.i_links_count++;
            pathToFile->dirty=1;

            iput(pathToFile);

        }
        else
        {
            printf("The des file exists!\n");
            pathToFile->refCount--;
        }
    }
}

truncates(MINODE *mip)
{
    deallocAllBLK(running->cwd->dev, mip);
    mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L);
    mip->INODE.i_size=0;
    mip->dirty=1;
    mip->refCount++;
    iput(mip);
    //mip->refCount=1;
}

my_unlink()
{
    MINODE *mip = iget(running->cwd->dev, getino(running->cwd->dev, pathname, 0));
    mip->INODE.i_links_count--;
    char sname[128], line[128];
    MINODE *pip = getUpperRelatedParentOf(mip);

    if(mip->INODE.i_links_count<=0)
    {
        truncates(mip);
    }

    strcpy(line, pathname);
    strcpy(sname, basename(line));

    findmyname(pip, mip->ino, &sname);
    //printf("sname = %s\n",sname);
    rm_child(pip, sname, 0);

}

symlinks(char *path, char par[ ])
{

    char oldNAME[60];
    char newNAME[60];

    if(strlen(path) > 60 || strlen(par) > 60 )
    {
        printf("Cmon man I only got 60 chars TT \n");
        return;
    }//pwd_helper

    memset(oldNAME, 0, sizeof(oldNAME));
    memset(newNAME, 0, sizeof(newNAME));

    strncpy(oldNAME, path, sizeof(oldNAME));
    strncpy(newNAME, par, sizeof(newNAME));

    //printf("oldname = %s\n", oldNAME);
    //printf("newName = %s\n", newNAME);

    int ino = getino(running->cwd->dev, oldNAME, -1);
    char *b = basename(path);

    if(ino != -1)
    {
        int alloc = ialloc(running->cwd->dev);
        mymk(1, newNAME, alloc, 0, 0);

        MINODE *mip = iget(running->cwd->dev, alloc);
        MINODE *ip = customizeTtestSearch(running->cwd->dev, oldNAME);

        char name[60];
        memset(name, 0, sizeof(name));

        pwd_helper(getUpperRelatedParentOf(ip), ip, &name);

        int x = strlen(b);
        int y = strlen(name);

        int i;
        for(i = 0; i < x; i++)
        {
            name[i+y] = b[i];
        }

        //printf("The name is: %s\n", name);

        mip->INODE.i_mode=0xA1A4;
        mip->INODE.i_size=strlen(name);
        memcpy(mip->INODE.i_block, name, strlen(name));

        mip->dirty=1;

        iput(mip);

    }
    else
    {
        printf("Failed because the source file does not exist!!\n");
    }

}

my_symlink()
{
    symlinks(pathname, parameter);
}

rm_file()
{
    rm_helper(0);
}

rm_dir()
{
    rm_helper(1);
}

int rm_helper(int dir)
{
    int ok;
    //mountptr?
    int ino = getino(running->cwd->dev, pathname, dir);

    //printf("in rm_helper ino : %d\n",ino);// pathname);
    if(ino == -1)
    {
        return -1;

    }

    MINODE *mip = iget(running->cwd, ino);

    char sname[128];
    char *pname, line[128];
    strcpy(line, pathname);
    pname = basename(line);
    strcpy(sname, pname);

    MINODE *pip;

    //exclude root
    //but re-concern it if mount
    if(mip->ino == root->ino){
        printf("Cannot rm root dir!\n");
        mip->refCount--;
        return -1;
    }

    if(dir==0)
    {
        ok=1;
    }
    else if(dir==1 && (checkDirIsEmpty(mip) == 1))
    {
        ok=1;
    }

    if((running->uid == SUPER_USER | (mip->INODE.i_uid == running->uid)) && mip->refCount == BUSY &&  ok==1)
    {
        //printf("mip all link count = %d, belongs to ino=%d\n", mip->INODE.i_links_count, mip->ino);
        //if(mip->INODE.i_links_count <= 0)
        //{
            //printf("Delete all\n");
        if(S_ISLNK(mip->INODE.i_mode)==0)
        {
            int i;
            for (i=0; i<12; i++){
                if (mip->INODE.i_block[i]==0)
                    continue;
                bdealloc(mip->dev, mip->INODE.i_block[i]);
            }
        }
        else
        {
            /*
             * I really don't know how to dealloc the block assigned to the soft link, because
             * if we use its block to store the referenced file's path then we lose the pointers point to
             * the block we assigned to it previously*/
            //bdealloc(mip->dev, (u32)mip->INODE.i_block[0]);

            //Oops, never mind, no need to assign new block to soft link
        }

            idealloc(mip->dev, mip->ino); //Work on it !!!!!!

            iput(mip); //(which clears mip->refCount = 0);

        //}

        pip = getUpperRelatedParentOf(mip);
        //printf("in rm_helper parent : %d\n",pip->ino);// pathname);
        pip->refCount++;

        //printf("The parent is %d\n", pip->ino);

        findmyname(pip, mip->ino, &sname);

        //printf("Sname = %s\n", sname);
        rm_child(pip, sname, dir);

        pip->INODE.i_atime=pip->INODE.i_ctime=pip->INODE.i_mtime = time(0L);
        pip->dirty=1;
        pip->INODE.i_links_count--;

        pip->refCount++;

        iput(pip);

        return 1;
    }

    if((running->uid == SUPER_USER | (mip->INODE.i_uid == running->uid))==0)
    {
        printf("User id is wrong\n");
    }
    if(mip->refCount != BUSY)
    {
        printf("The target is busy, had been accessing by %d processes \n", running->cwd->refCount);
    }
    if(dir==1 && (checkDirIsEmpty(mip) == 0))
    {
        printf("The target dir is not empty\n");
    }

    mip->refCount--;
    return -1;
}

// rm_child(): removes the entry [INO rlen nlen name] from parent's data block.

rm_child(MINODE *parent, char *name, int dir)
{
    //printf("*************************\nIt is looking for %s\n*******************************\n", name);
    MINODE *mip = getUnderKneesChildOf(parent, name, dir);

    char temp[128];

    if(mip != NULL)
    {
        //printf("The child is %d\n", mip->ino);

        char buf[BLKSIZE];
        int index=0;
        char *cp, *cwd;
        DIR *dp;
        char *rm = NULL;
        int find = 0;
        int len = 0;

        while(parent->INODE.i_block[index]!=0)
        {
            get_block(parent->dev, parent->INODE.i_block[index], &buf);

            cp = buf;
            dp = (DIR *)buf;

            while(cp+dp->rec_len < buf + BLKSIZE != 0)
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = 0;

                if(find == 0 && dp->inode == mip->ino && ((strlen(temp) == 1 && temp[0] == '.') == 0 && strcmp(temp, "..") != 0) && strcmp(temp, name) == 0)
                {
                    find = 1;
                    len = dp->rec_len;
                    rm = cp;
                    //printf("removing %s\n", dp->name);

                }
                else if(find == 1)
                {

                    DIR *tmp = (DIR *)rm;
                    tmp->file_type=dp->file_type;
                    tmp->inode=dp->inode;
                    strcpy(tmp->name,temp);
                    tmp->name_len=dp->name_len;
                    tmp->rec_len=dp->rec_len;

                    rm += tmp->rec_len;
                    //printf("move up = %s\n", tmp->name);
                }

                    cwd=cp;
                    cp+=dp->rec_len;
                    dp=(DIR *)cp;


            }

            //if rm down 1
            if(rm==NULL)
            {
                //printf("rm is NULL\n");
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = 0;

                if(dp->inode == mip->ino && ((strlen(temp) == 1 && temp[0] == '.') == 0 && strcmp(temp, "..") != 0) && strcmp(temp, name) == 0)
                {
                    if(dp->rec_len==BLKSIZE)
                    {
                        //int i = parent->INODE.i_block[index];

                        while(parent->INODE.i_block[index] !=0 && index < 11)
                        {
                            parent->INODE.i_block[index] = parent->INODE.i_block[index+1];
                            index++;
                        }

                        parent->INODE.i_block[11]=0;

                        //put_block(parent->dev, i, &buf);
                        parent->dirty=1;
                        iput(parent);
                        return;
                    }
                    else
                    {
                        DIR *tmp;

                        tmp = (DIR *)cwd;

                        tmp->rec_len+=dp->rec_len;
                        //printf("removing last dir %s\tmoving up %s\n", dp->name, tmp->name);

                        put_block(parent->dev, parent->INODE.i_block[index], &buf);
                        parent->dirty=1;
                        iput(parent);
                        return;
                    }

                }
            }
            //if rm down 2
            else
            {


                    //printf("rm is not NULL\n");

                    strncpy(temp, dp->name, dp->name_len);
                    temp[dp->name_len] = 0;

                    DIR *tmp = (DIR *)rm;
                    tmp->file_type=dp->file_type;
                    tmp->inode=dp->inode;
                    strcpy(tmp->name,temp);
                    tmp->name_len=dp->name_len;
                    tmp->rec_len+=dp->rec_len;

                    //printf("move up = %s\n", tmp->name);

                    put_block(parent->dev, parent->INODE.i_block[index], &buf);
                    parent->dirty=1;
                    iput(parent);
                    return;

            }

            index++;
        }


    }
    else
    {
        printf("No such dir\n");
    }

}

creat_file()
{
    mymk(0, pathname, ialloc(running->cwd->dev), balloc(running->cwd->dev), 0);
}

make_dir()
{
    mymk(1, pathname, ialloc(running->cwd->dev), balloc(running->cwd->dev), 0);
}

cd()
{
    int dev = running->cwd->dev;

    if (pathname){

        int tmp = testSearch(dev, pathname, 1, 1);

        if(tmp > -1)
        {
            //should ref when cd?
            running->cwd->refCount--;
            running->cwd=iget(dev, tmp);
        }
    }
}


ls()
{
      MINODE *mip = NULL;
      MINODE *tmp;

      char buf[BLKSIZE];
      //char *tar=NULL;
      int dev;
      char *cp;
      char temp[256];
      DIR *dp;

      int index = 0;

      if (pathname){   // ls pathname:

           printf("=================================================================\n");

           if (pathname[0]=='/')
           {
               dev = root->dev;
           }
           else
           {
               dev = running->cwd->dev;
           }

          int t = testSearch(dev, pathname, 0, 1);

          if(t <= -1)
          {
            printf("can't find %s, BOMB OUT!\n", pathname);
            return;
          }

          mip = iget(dev, t);
          mip->refCount--;
              //Need to handle indirect(s)
          //printf("i_number  rec_len  name_len  name\n");
          while(mip->INODE.i_block[index]!=0)
          {
              get_block(dev, mip->INODE.i_block[index], &buf);     // read INODE's i_block[0]

              dp=(DIR *)buf;
              cp = buf;


              while(cp+dp->rec_len < buf +BLKSIZE != 0 ){
                   strncpy(temp, dp->name, dp->name_len);
                   temp[dp->name_len] = 0;

                   tmp = iget(dev, dp->inode);
                   tmp->refCount--;

                   //printf("%d  \t  %d  \t   %d  \t     %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
                   ls_dir(tmp, temp);

                   cp += dp->rec_len;

                   dp = (DIR *)cp;


              }

              strncpy(temp, dp->name, dp->name_len);
              temp[dp->name_len] = 0;

              tmp = iget(dev, dp->inode);
              tmp->refCount--;
              //printf("%d  \t  %d  \t   %d  \t     %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
              ls_dir(tmp, temp);


              index++;
        }

          printf("=================================================================\n");
    }
}


ls_dir(MINODE *st, char *name)
{
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";

  int i;
  char ftime[64];

  if ((st->INODE.i_mode & 0xF000) == 0x8000)
    printf("-");
  if ((st->INODE.i_mode & 0xF000) == 0x4000)
    printf("d");
  if ((st->INODE.i_mode & 0xF000) == 0xA000)
    printf("l");

  for (i=8; i >= 0; i--){
    if (st->INODE.i_mode & (1 << i))
      printf("%c", t1[i]);
    else
      printf("%c", t2[i]);
  }

  printf("%4d ",st->INODE.i_links_count);
  printf("%4d ",st->INODE.i_gid);
  printf("%4d ",st->INODE.i_uid);
  printf("%8d ",st->INODE.i_size);

  // print time
  //printf("%s",ctime(&st.i_dtime));
  strcpy(ftime, ctime(&st->INODE.i_dtime));
  ftime[strlen(ftime)-9] = 0;
  printf("%s ",ftime);

  // print name

  printf("%s", name);

  if(S_ISLNK(st->INODE.i_mode))
  {
      char buff[60];
      memset(buff, 0, sizeof(buff));

      char *a = (char *)st->INODE.i_block;
      strcpy(buff, a);
      char *b = basename(buff);
      //printf("symlink name = s\n", b);
      printf(" -> %s", b);
  }

  printf("\n");

}

cat_file()
{
     char mybuf[BLKSIZE];
     strcpy(parameter, "R");
     int fd = open_file_helper();

     if(S_ISLNK(running->fd[fd]->inodeptr->INODE.i_mode))
     {
         char buff[60];
         //char buf[60], buff[BLKSIZE];
         //char *f = fd+'0';
         //char b[23]="/lost+found/reading/fdx";
         //b[22]=f;

         //read_link(b, &buf);//buf = path to symlink itself
         //printf("path to symlink itself = %s\npath in symlink's block = %s\n", buf, buff);

         read_link((char *)running->fd[fd]->inodeptr->INODE.i_block, &buff); //buff = path in symlink's blok

         //printf("the linked path is '%s'\n", buff);

         strcpy(pathname, buff);
         strcpy(parameter, "R");

         cat_file();
     }
     else
     {

         while(myread(fd, &mybuf, BLKSIZE, 1)){ }

     }

     printf("\n");
     close_file_helper(fd);
}
