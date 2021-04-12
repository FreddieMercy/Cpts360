#include "vars.h"

// function proto types
int falloc()
{

    int i;

    for(i=0;i<10;i++)
    {
        if (running->fd[i]==0)
        {
            return i;
        }
        //printf("running->fd[%d]=%d\n", i, running->fd[i]);
    }

    return -1;

}

chkFd(MINODE *mip)
{
    int i;

    for(i =0;i<10;i++)
    {
        //printf("mip->fd[%d]=%d\n", i, running->fd[i]);
        if (running->fd[i] != 0)
        {
            if (running->fd[i]->inodeptr == mip)
            {
                if (running->fd[i]->mode>0)
                {
                    printf("Error: File already opened for writing\n");
                    return -2;
                }

                return i;
            }
        }
    }

    return -1;
}

int transMode(char *cmode)
{

    if(strcmp(cmode, "R")==0)
    {
        return 0;
    }
    if(strcmp(cmode, "W")==0)
    {
        return 1;
    }
    if(strcmp(cmode, "RW")==0)
    {
        return 2;
    }
    if(strcmp(cmode, "APPEND")==0)
    {
        return 3;
    }

    return -1;
}

MINODE *iget(int dev, int ino)
{
    char buf[BLKSIZE];
    int i=0, freeNode=0, block=0, pos=0;
    freeNode=0;
    for (i = 0; i < NMINODES; i++) {
        if(minode[i].ino == 0){
            if(freeNode == 0)
                freeNode = i;
        }
        if(minode[i].ino == ino && minode[i].dev==dev){
            minode[i].refCount ++;
            //printf("dev = %d\tino-%d\n", minode[i].dev, minode[i].ino);
            //printf("-------------------------------------------------\n");
            return &minode[i];
        }

        //printf("dev = %d\tino-%d\n", minode[i].dev, minode[i].ino);
    }

    GD *gps;
    get_block(dev, 2, &buf);
    gps = (GD *)buf;

    block = (ino-1) / INODES_PER_BLOCK + gps->bg_inode_table;
    pos = (ino-1) % INODES_PER_BLOCK;

    get_block(dev, block, &buf);

    minode[freeNode].dev = dev;
    minode[freeNode].dirty = 0;
    minode[freeNode].ino = ino;
    memcpy(&minode[freeNode].INODE, (((INODE*)buf) + pos),sizeof(INODE));
    minode[freeNode].mounted = 0;
    minode[freeNode].mountptr = NULL;
    //minode[freeNode].mountptr=root->mountptr;
    minode[freeNode].refCount = 1;

    return &minode[freeNode];
}

int getino(int *dev, char *path, int dir)
{
    return search(dev, path, dir);
}

//Just a decent wrapper
int search(int dev, char *path, int dir)
{
    //dir = -1 means don't care
    return testSearch(dev, path, 0, dir);
}

//disamble to dir and filename
int testSearch(int *dev, char *path, int cd, int dir)
{

    if(path)
    {
        int find;
        char line1[128], line2[128];
        MINODE *mip;

        strcpy(line1, path);
        strcpy(line2, path);

        char *d, *b;

        d=dirname(line1);
        b=basename(line2);

        //printf("d = %s\n", d);
        //printf("b = %s\n", b);

        //printf("strlen(path) = %d\n", strlen(path));

        if (path[0]=='/')
        {
           mip = root;

        }
        else
        {
            mip = running->cwd;
        }


        if(d != NULL && (d[0]!='.' | strlen(d) > 1))
        {

            find = testSearchParser(dev, d, 1, mip);


            if(find==-1)
            {
                return -1;
            }

            mip = iget(dev, find);
            mip->refCount--;

        }
        else
        {
            mip = running->cwd;
        }


        if(b != NULL && (b[0]=='.' & strlen(b) == 1 & cd == 1 & strlen(path)==0))
        {
            return root->ino;
        }

        find = testSearchParser(dev, b, dir, mip);

        if(find == 0)
        {
            return -1;
        }

        return find;
    }

    return -1;

}

//Search, most important
int testSearchFindit(int dev, char *name, int dir, MINODE *mip)
{
        char buf[BLKSIZE];

        MINODE *tmps;
        char *cp;
        char temp[256];

        int index = 0;

        //need to handle indirect(s)
        while(mip->INODE.i_block[index] != 0 && index < 12)
        {
            get_block(dev, mip->INODE.i_block[index], &buf);     // read INODE's i_block[0]

            cp = buf;
            DIR *dp = (DIR *)buf;


            while(cp+dp->rec_len < buf + BLKSIZE != 0){

                 strncpy(temp, dp->name, dp->name_len);
                 temp[dp->name_len] = 0;

                 tmps = iget(dev, dp->inode);
                 tmps->refCount--;

                 if(
                         (strcmp(name, temp)==0) &&
                         (
                             (dir == 1 && S_ISDIR(tmps->INODE.i_mode)) |
                             (dir == 0 && S_ISDIR(tmps->INODE.i_mode) == 0) |
                             (dir < 0)
                         )

                    )
                {

                        return dp->inode;

                }


                 // move to the next DIR entry:
                 cp += dp->rec_len;   // advance cp by rec_len BYTEs
                 dp = (DIR *)cp;     // pull dp along to the next record
            }

            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;

            tmps = iget(dev, dp->inode);
            tmps->refCount--;
            //printf("dir = %d\tname=%s\ttemp=%s\n", dir, name, dp->name);

            if(
                    (strcmp(name, temp)==0) &&
                    (
                        (dir == 1 && S_ISDIR(tmps->INODE.i_mode)) |
                        (dir == 0 && S_ISDIR(tmps->INODE.i_mode) == 0) |
                        (dir < 0)
                    )

               )

            {

                    return dp->inode;

            }

            index++;

        }

        return 0;

}

//Parse based on cd and dir
int testSearchParser(int *dev, char *path, int dir, MINODE *ip)
{

    if (path){   // ls pathname:

        if(path != NULL && (path[0]!='.' | strlen(path) > 1))
        {
            char *tar = strtok(path,"/");

            if(tar != NULL)
            {

                u32 inumber;

                while(tar != NULL)
                {
                    inumber = testSearchFindit(dev, tar, dir, ip);

                    if(inumber == 0)
                    {
                        //printf("can't find %s, BOMB OUT!\n", tar);
                        return -1;
                    }


                    tar=strtok(NULL,"/");

                    ip=iget(dev,inumber);
                    ip->refCount--;
                }

                return inumber;
            }
            else
            {
                return root->ino;
            }
        }
        else
        {
            return ip->ino;
        }

  }
}

int findCmd(char *command)
{
   int i = 0;
   while(comd[i]){
     if (strcmp(command, comd[i])==0)
         return i;
     i++;
   }
   return -1;
}

MINODE *customizeTtestSearch(int *dev, char *path)
{

    if(path)
    {
        int find;
        char line1[128];
        MINODE *mip;

        strcpy(line1, path);

        char *d;

        d=dirname(line1);

        //printf("d = %s\n", d);
        //printf("b = %s\n", b);

        //printf("strlen(path) = %d\n", strlen(path));

        if (path[0]=='/')
        {
           mip = root;

        }
        else
        {
            mip = running->cwd;
        }


        if(d != NULL && (d[0]!='.' | strlen(d) > 1))
        {

            find = testSearchParser(dev, d, 1, mip);


            if(find==-1)
            {
                return NULL;
            }

            mip = iget(dev, find);
            mip->refCount--;

        }
        else
        {
            mip = running->cwd;
        }

        return mip;
    }

    return NULL;

}
