#include "headers/KC_Code.h"

MOUNT *getmountp(char *name)
{
    int i;
    for(i=0; i <NMOUNT; i++)
    {
        if(mounttab[i].busy!=BUSY)
        {
            //printf("Selected %d\n", i);
            return &mounttab[i];
        }
        if(strcmp(mounttab[i].name, name) == 0)
        {
            printf("Already mounted!\n");
            return NULL;
        }
    }

    printf("Cannot mount any more!!\n");
    return NULL;

}

int checkDirIsEmpty(MINODE *mip) //No type check, hence be careful!!
{
    if(mip->INODE.i_links_count>2)
    {
        return 0;
    }

    char buf[BLKSIZE];

    int dev;
    char *cp;
    char temp[256];
    DIR *dp;


    dev = mip->dev;

            get_block(dev, mip->INODE.i_block[0], &buf);     // read INODE's i_block[0]

            dp=(DIR *)buf;
            cp = buf;

            while(cp+dp->rec_len < buf +1024 != 0 ){
                 strncpy(temp, dp->name, dp->name_len);
                 temp[dp->name_len] = 0;

                 //not sure strcmpy works correctly with "."
                 if((strlen(temp) == 1 && temp[0] == '.') == 0 && strcmp(temp, "..") != 0)
                 {
                     return 0;
                 }


                 cp += dp->rec_len;

                 dp = (DIR *)cp;


            }

            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;

            if((strlen(temp) == 1 && temp[0] == '.') == 0 && strcmp(temp, "..") != 0)
            {
                return 0;
            }

            return 1;
}

findmyname(MINODE *parent, int myino, char *myname)
{
    char buf[BLKSIZE];
    int index=0;
    char *cp;
    DIR *dp;
    char temp[128];
    char tar[128];// = NULL;

    //printf("Parent = %d\n", parent->ino);

    while(parent->INODE.i_block[index]!=0)
    {
        get_block(parent->dev, parent->INODE.i_block[index], &buf);

        cp = buf;
        dp = (DIR *)buf;

        while(cp+dp->rec_len < buf + BLKSIZE != 0)
        {
            if(dp->inode == myino)
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = 0;

                if(dp->inode == myino && ((strlen(temp) == 1 && temp[0] == '.') == 0 && strcmp(temp, "..") != 0))
                {

                    if(strcmp(myname, temp)==0)
                    {
                        //printf("Found filename is %s", myname);
                        return;
                    }
                    strcpy(tar, temp);

                }


            }
            //printf("in find my name the NAME is :%s\n",myname);

            cp+=dp->rec_len;
            dp=(DIR *)cp;
        }

        if(dp->inode == myino)
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;

            if(dp->inode == myino && ((strlen(temp) == 1 && temp[0] == '.') == 0 && strcmp(temp, "..") != 0))
            {

                if(strcmp(myname, temp)==0)
                {
                    //printf("Found filename is %s", myname);
                    return;
                }

                strcpy(tar, temp);
            }
        }
        //printf("in find my name the NAME is :%s\n", myname);

        index++;
    }

    strcpy(myname, tar);
}

MINODE *getUpperRelatedParentOf(MINODE *mip)
{
    if(S_ISDIR(mip->INODE.i_mode))// == 0)
    {
        //printf("mip->ino== %d\n", mip->ino);
        char buf[BLKSIZE];
        MINODE *pip;
        char *cp;
        DIR *dp;


        get_block(mip->dev, mip->INODE.i_block[0], (char *)&buf);

        dp = (DIR *)buf; // get first dir "."
        if(mip==root)// || dp->rec_len>BLKSIZE)
        {
            //printf("Root\n");
            return root;
        }
        else
        {
            //printf(". = %s and its rec_len = %d\n", dp->name, dp->rec_len);
            cp = buf + dp->rec_len;
            dp = (DIR *)cp; // get second dir ".."

            //printf(".. = %s\n", dp->name);

            //printf("mip->dev : %d\tdp->inode : %d\n", mip->dev, dp->inode);
            pip = iget(mip->dev, dp->inode);
            pip->refCount--;

            return pip;
        }
    }
    else
    {

        char pwd[BLKSIZE];//, buff[BLKSIZE];

        strcpy(pwd, pathname);

        char *d = dirname(pwd);

        //printf("the dirname is %s\n", d);

        MINODE *tmp = iget(running->cwd->dev, getino(running->cwd->dev, d, 1));
        tmp->refCount--;

        //printf("the target file's parent is: %d\n", tmp->ino);
        return tmp;
    }
}

MINODE *getUnderKneesChildOf(MINODE *pip, char *name, int dir)
{
    MINODE *mip;
    /*
    if((strlen(name) == 1 && temp[0] == '.') | strcmp(temp, "..") == 0)
    {
        printf("Please don't take the Parent and Grand Parent of cwd\n");
        return NULL;
    }
    */


    //printf("[In getChild] looking for chil : %s\n", name);

    int ino = testSearchFindit(pip->dev, name, dir, pip);

    if(ino == -1)
    {
        printf("Did not find the child from getUnderKneesChildOf(MINODE *pip, char *name, int dir)\n");
        return NULL;
    }

    mip = iget(pip->dev, ino);
    mip->refCount--;

    return mip;
}


int iput(MINODE *mip)//This function releases a Minode[] pointed by mip.

{
    char buf[BLKSIZE];

    int block, pos;
    mip->refCount--;
    INODE *itmp;

    if(mip->refCount > 0)
        return;

    /*
    if(mip->refCount > 1)
        return;
        */
    if(mip->dirty == 0)
        return;

    GD *gps;
    get_block(mip->dev, 2, &buf);
    gps = (GD *)buf;

    block = (mip->ino - 1) / INODES_PER_BLOCK + gps->bg_inode_table;
    pos = (mip->ino - 1) % INODES_PER_BLOCK;

    itmp = (INODE *)buf + pos;

    get_block(mip->dev, block, &buf);
    memcpy( itmp, &mip->INODE, 128 );
    put_block(mip->dev, block, &buf);

    return;

}

int mymk(int dir, char *path, int ino, int block, int ln)
{
    //printf("block = %d\n", block);
    int find;
    char buf[BLKSIZE];

    char *d, *b;
    char line[128], line1[128];
    DIR *dps;
    char *cp;

    int i;

    MINODE *tmp;

    int dev = running->cwd->dev;

    if (path){

        strcpy(line, path);
        strcpy(line1, path);
        d = dirname(line);
        b = basename(line1);
        if (path[0]=="/")
        {
           dev = root->dev;
        }

        find = testSearch(dev, path, 0, dir);

        if(find != -1)
        {
            printf("File exists!\n");
            return -1;
        }

        find = testSearch(dev, d, 0, 1);

        if(find==-1)
        {
            printf("Wrong Dir!\n");
            return -1;
        }

        tmp = iget(dev, find);

        //printf("block = %d\n", block);
        //printf("inode = %d\n", ino);

        MINODE *mip = iget(dev, ino);

        if(ln == 0)
        {
            mip->INODE.i_block[0] = block;

            for (i = 1; i < 16; i++) {
                mip->INODE.i_block[i]=0;
            }

            mip->dirty=1;
            mip->ino = ino;
            if(dir==1)
            {
                mip->INODE.i_mode=0x41ED;
                mip->INODE.i_links_count=2;
                mip->INODE.i_size=1024;
            }
            else
            {
                mip->INODE.i_mode=0x81A4;
                mip->INODE.i_links_count=1;
                mip->INODE.i_size=0;
            }

            mip->INODE.i_uid=running->uid;
            mip->INODE.i_gid=running->gid;


            mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L);
            mip->INODE.i_blocks=2;
            iput(mip);
        }
        else
        {
            mip->refCount--;
        }

            //FILE?
            //if(dir == 1)
            //{
                get_block(dev, block, &buf);
                cp = buf;
                dps = (DIR *)buf;
                dps->inode = ino;
                strcpy(dps->name, ".");
                dps->name_len = 1;
                dps->rec_len = 12;


                cp += dps->rec_len;
                dps = (DIR *)cp;

                dps->inode = tmp->ino;
                dps->name_len = 2;
                strcpy(dps->name, "..");
                dps->rec_len = BLOCK_SIZE - 12;

                put_block(dev, block, &buf);
            //}

            get_block(tmp->dev, tmp->INODE.i_block[0], &buf);

            dps = (DIR *)buf;

            cp = buf;

            while(cp+dps->rec_len < buf +BLKSIZE) {

                cp += dps->rec_len;
                dps = (DIR *)cp;

            }

            // calculate size of last item in cwd to reduce rec_len
            int need_length = 4*((8+dps->name_len+3)/4);
            // storing the lenght of the new last dir
            int len = dps->rec_len - need_length;

            // change last dir rec_len to needed length
            if (len>=need_length) //check to see if we need to allocate another data block
            {

                // change last dir rec_len to needed length
                dps->rec_len = need_length;

                cp += dps->rec_len;
                dps = (DIR *)cp;

                dps->rec_len = len;

                //printf("******************\nb=%s\tdps->name=%s\n***********************\n", b, dps->name);

                dps->name_len = strlen(b);
                dps->inode = mip->ino;
                strcpy(dps->name, b);

                put_block(tmp->dev, tmp->INODE.i_block[0], &buf);

            }
            else
            {
                i = 0;
                while (len<need_length){
                    i++;
                    if (tmp->INODE.i_block[i]==0)
                    {
                        tmp->INODE.i_block[i]=balloc(dev);
                        tmp->refCount = 0;
                        len = 1024;
                        get_block(dev, tmp->INODE.i_block[i], &buf);
                        cp = buf;
                        dps = (DIR *) buf;
                    }
                    else{
                        get_block(dev,tmp->INODE.i_block[i], &buf);
                        cp = buf;
                        dps = (DIR *) buf;

                        while(cp+dps->rec_len < buf +BLKSIZE)
                        {
                            cp += dps->rec_len;
                            dps = (DIR *)cp;
                        }
                        // calculate size of last item in cwd to reduce rec_len
                        need_length = 4*((8+dps->name_len+3)/4);
                        // storing the lenght of the new last dir
                        len = dps->rec_len - need_length;
                        if (len>=need_length)
                        {
                            dps->rec_len = need_length;
                            cp += dps->rec_len;
                            dps = (DIR *)cp;
                        }

                    }
                }

                dps->rec_len = len;
                dps->name_len = strlen(b);
                dps->inode = mip->ino;
                strcpy(dps->name, b);


                put_block(tmp->dev, tmp->INODE.i_block[i], &buf);
            }


            tmp->dirty = 1;

            if(dir == 1)
            {
                tmp->INODE.i_links_count++;
            }


            tmp->INODE.i_atime = time(0);
            iput(tmp);
            return 1;

    }
}

pwd()
{
    char sname[128];
    memset(sname, 0, sizeof(sname));
    pwd_helper(getUpperRelatedParentOf(running->cwd),running->cwd, sname);
    printf("%s\n", sname);
}

pwd_helper(MINODE *mip, MINODE *ip, char *name)
{
    char sname[128];
    memset(sname, 0, sizeof(sname));

    int i =0;

    if(mip->ino==root->ino & ip->ino==root->ino)
    {
        int z = strlen(name);

        for(i=z; i>0; i--)
        {
            name[i] = name[i-1];
            //printf("%c", name[i]);
        }
        name[0]='/';
    }/*
    else if(mip->ino==root->ino & ip->ino!=root->ino)
    {
        findmyname(mip, ip->ino, &sname);

        sname[strlen(sname)]='/';

        int x = strlen(sname);

        int y = strlen(sname) + strlen(name);

        for(i= x; i < y;i++)
        {
            sname[i] = name[i-x];
            printf("%c", name[i-strlen(sname)]);
        }

        strcpy(name, sname);

        int z = strlen(name);

        for(i=z; i>0; i--)
        {
            name[i] = name[i-1];
            //printf("%c", name[i]);
        }
        name[0]='/';
    }*/

    else
    {
        findmyname(mip, ip->ino, &sname);

        sname[strlen(sname)]='/';

        int x = strlen(sname);

        int y = strlen(sname) + strlen(name);

        for(i= x; i < y;i++)
        {
            sname[i] = name[i-x];
            printf("%c", name[i-strlen(sname)]);
        }

        strcpy(name, sname);

        pwd_helper(getUpperRelatedParentOf(mip),mip, name);
    }

}


deallocAllBLK(int dev, MINODE *pip)
{
    char buf[BLKSIZE];
    int i = 0, j = 0;

    char bitmap[BLKSIZE];

    int indBLK;
    int dbindBLK;
    u64 *ind,*dbind;

    char dbbuf[BLKSIZE];

    get_block(dev,BBITMAP,&bitmap);

    MINODE *mip = NULL;
    if(S_ISLNK(pip->INODE.i_mode) == 0)
    {
        mip = pip;
    }
    else
    {
        char dir[60];
        read_link((char *)pip->INODE.i_block, &dir);
        mip=iget(running->cwd->dev, getino(running->cwd->dev, dir, 0));
        mip->refCount--;
    }

    for ( i = 0; i<12; i++)
    {
        if (mip->INODE.i_block[i]!=0)
        {

            clr_bit(bitmap, mip->INODE.i_block[i]);

            mip->INODE.i_block[i]=0;

        }

        else
        {

            put_block(dev,BBITMAP,&bitmap);

            return;

        }

        }

    // on to ind blocks
    if (mip->INODE.i_block[i]!=0)
    {
        indBLK = mip->INODE.i_block[i];
        get_block(dev,indBLK,&buf);
        ind = (unsigned long *)buf;
        for (i=0;i<256;i++)
        {
            if(*ind != 0)
            {
                clr_bit(bitmap, *ind);
                *ind = 0;
                ind++;
            }
            else
            {
                clr_bit(bitmap, indBLK);
                put_block(dev,indBLK,&buf);
                put_block(dev,BBITMAP,&bitmap);
                mip->INODE.i_block[12] = 0;
                return;
            }
        }
    }
    else
    {
        put_block(dev,BBITMAP,&bitmap);
        return;
    }
    //then double ind
    if (mip->INODE.i_block[13]!=0)
    {
        dbindBLK = mip->INODE.i_block[13];
        get_block(dev,dbindBLK,&dbbuf);
        dbind = (u64 *)dbbuf;
        for (i=0;i<256;i++)
        {
            indBLK = *dbind;
            get_block(dev,indBLK,&buf);
            ind = (u64 *)buf;
            for (j=0;j<256;j++)
            {
                if(*ind != 0)
                {
                    clr_bit(bitmap, *ind);
                    *ind = 0;
                    ind++;
                }
                else
                {
                    clr_bit(bitmap, indBLK);
                    clr_bit(bitmap, dbindBLK);
                    put_block(dev,indBLK,&buf);
                    put_block(dev,BBITMAP,&bitmap);
                    put_block(dev,dbindBLK,&dbbuf);
                    mip->INODE.i_block[13] = 0;
                    return;
                }
                clr_bit(bitmap, indBLK);

            }
            dbind++;
            if (*dbind == 0)
            {//edge case handling
                clr_bit(bitmap, indBLK);
                clr_bit(bitmap, dbindBLK);
                put_block(dev,indBLK,&buf);
                put_block(dev,BBITMAP,&bitmap);
                put_block(dev,dbindBLK,&dbbuf);
                mip->INODE.i_block[13] = 0;
                return;
            }
        }
    }
    else
    {
        put_block(dev,BBITMAP,&bitmap);
        return;
    }

}

menu()
{

    printf("******************** Menu *******************\n");
    printf("mkdir     creat     mount     umount    rmdir\n");
    printf("cd        ls        pwd       stat      rm   \n");
    printf("link      unlink    symlink   chmod     chown  touch\n");
    printf("open      pfd       lseek     rewind    close\n");
    printf("read      write     cat       cp        mv\n");
    printf("cs        fork      ps        kill      quit\n");
    printf("=============   Usage Examples ==============\n");
    printf("mkdir  filename\n");
    printf("mount  filesys   /mnt\n");
    printf("chmod  filename  0644\n");
    printf("chown  filename  uid\n");
    printf("open   filename  mode (0|1|2|3 for R|W|RW|AP)\n");
    printf("write  fd  text_string\n");
    printf("read   fd  nbytes\n");
    printf("pfd    (display opened file descriptors)\n");
    printf("cs     (switch process)\n");
    printf("fork   (fork child process)\n");
    printf("ps     (show process queue as Pi[uid]==>}\n");
    printf("kill   pid   (kill a process)\n");
    printf("*********************************************\n");


}

my_stat()
{
    char ftime[64];
    strcpy(ftime, ctime(&running->cwd->INODE.i_dtime));
    ftime[strlen(ftime)-9] = 0;

    int i;
    int x = strlen(ftime);
    for(i = 4; i < x; i++)
    {
        ftime[i-4]=ftime[i];
    }
    ftime[x-4]=0;
    printf("\n", ftime);
    printf("*********  stat **********\n");
    printf("dev=%d   ino=%d   mod=%x\n", running->cwd->dev, running->cwd->ino, running->cwd->INODE.i_mode);
    printf("uid=%d   gid=%d   nlink=%d\n", running->cwd->INODE.i_uid, running->cwd->INODE.i_gid, running->cwd->INODE.i_links_count);
    printf("size=%d time=%s\n", running->cwd->INODE.i_size,ftime);
    printf("**************************\n");

}

quit()
{

    my_quit();
    exit(0);
}

my_quit()
{
    int i = 0;
    //put all dirty inodes
    for (i = 0; i<NMINODES;i++)
    {
        if (minode[i].ino != 0)
        {
            minode[i].refCount=1;
            iput(&minode[i]);
        }

    }
}
