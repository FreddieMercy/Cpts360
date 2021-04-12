/*	type.h for CS360 Project             */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ext2_fs.h"
#include <libgen.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdarg.h>

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

#define BLOCK_SIZE        1024
#define BLKSIZE           1024

#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define BBITMAP           3
#define IBITMAP           4
#define INODEBLOCK        5
#define ROOT_INODE        2

// Default dir and regulsr file modes
#define DIR_MODE          0040777
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

// Proc status
#define FREE              0
#define BUSY              1
#define KILLED            2

// Table sizes
#define NMINODES          50
#define NMOUNT            10
#define NPROC             10
#define NFD               10
#define NOFT              50


// Open File Table
typedef struct Oft{
  int   mode;
  int   refCount;
  struct Minode *inodeptr;
  long  offset;
} OFT;

// PROC structure
typedef struct Proc{
  int   uid;
  int   pid;
  int   gid;
  int   ppid;
  int   status;

  struct Minode *cwd;
  OFT   *fd[NFD];

  struct Proc *next;
  struct Proc *parent;
  struct Proc *child;
  struct Proc *sibling;
} PROC;

// In-memory inodes structure
typedef struct Minode{
  INODE INODE;               // disk inode
  int   dev, ino;
  int   refCount;
  int   dirty;
  int   mounted;
  struct Mount *mountptr;
  char     name[128];           // name string of file
} MINODE;

// Mount Table structure
typedef struct Mount{
  int  ninodes;
  int  nblocks;
  int  bmap;
  int  imap;
  int  iblock;
  int  dev, busy;
  struct Minode *mounted_inode;
  char   name[256];
  char   mount_name[64];
} MOUNT;

char *comd[]={
    "mkdir",
    "cd",
    "pwd",
    "ls",
    "mount",
    "umount",
    "creat",
    "rmdir",
    "rm",
    "open",
    "close",
    "read",
    "write",
    "cat",
    "cp",
    "mv",
    "pfd",
    "lseek",
    "rewind",
    "mystat",
    "pm",
    "menu",
    "access_file",
    "chmod",
    "chown",
    "cs",
    "fork",
    "ps",
    "kill",
    "quit",
    "do_touch","sync", "link", "unlink", "symlink","chgrp", "stat",0};

typedef unsigned char   u8;            // unsigned char
typedef unsigned short u16;           // unsigned short
typedef unsigned int   u32;           // unsigned int
typedef unsigned long  u64;           // unsigned long
/************************ globals *****************************/

MINODE *root;
char pathname[128], parameter[128], cwdname[128];
char name[128];
char names[128][256];

int  nnames;
char *rootdev = "disk", *slash = "/", *dot = ".";
//int iblock;

MINODE minode[NMINODES];
MOUNT  mounttab[NMOUNT];
PROC   proc[NPROC], *running;
OFT    oft[NOFT];

int DEBUG=0;
int nproc=0;

int put_block(int dev, int blk, char buf[ ])
{
  lseek(dev, (long)blk*BLKSIZE, 0);
  write(dev, buf, BLKSIZE);
}

int get_block(int dev, int blk, char buf[ ])
{
  lseek(dev, (long)blk*BLKSIZE, 0);
  read(dev, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];
  SUPER *sp;
  GD *gp;

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, &buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, &buf);

  get_block(dev, 2, &buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, &buf);
}

int ialloc(int dev)
{
  //printf("In ialloc dev = %d\n", dev);
  int  i;
  char buf[BLKSIZE];

  get_block(dev, 2, &buf);
  GD *gp = (GD *)buf;

  int ninode = root->mountptr->ninodes;
  int imap = gp->bg_inode_bitmap;

  // read inode_bitmap block
  get_block(dev, imap, &buf);

  for (i=0; i < ninode; i++){

    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, &buf);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}
// balloc wrote based on ialloc, poor knock out
int decFreeBlocks(int dev)
{
    char buf[BLKSIZE];
    SUPER *sp;
    GD *gp;
    // dec free inodes count in SUPER and GD
    get_block(dev, 1, &buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count--;
    put_block(dev, 1, &buf);

    get_block(dev, 2, &buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count--;
    put_block(dev, 2, &buf);
}

int balloc(int devs)
{
    //printf("In balloc dev = %d\n", dev);
    char bufz[BLKSIZE];
    int  i;

    GD *gpz=NULL;

    int nblock=0;
    int bmap=0;

    get_block(devs, 2, &bufz);
    gpz = (GD *)bufz;
    nblock = root->mountptr->nblocks;
    bmap = gpz->bg_block_bitmap;
    // read inode_bitmap block

    get_block(devs, bmap, &bufz);

    for (i=0; i < nblock; i++){

      if (tst_bit(bufz, i)==0){
         set_bit(bufz,i);
         decFreeBlocks(devs);

         put_block(devs, bmap, &bufz);

         return i;
      }
    }
    printf("balloc(): no more free inodes\n");
    return 0;
}


int incFreeInodes(int dev)
{
  char buf[BLKSIZE];
  SUPER *sp;
  GD *gp;

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, &buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, &buf);

  get_block(dev, 2, &buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, &buf);
}

idealloc(int dev, int blk)
{
  char buf[BLKSIZE];
  get_block(dev, 2, &buf);
  GD *gp = (GD *)buf;
  int imap = gp->bg_inode_bitmap;

  // read inode_bitmap block
  get_block(dev, imap, &buf);

       clr_bit(buf,blk-1);
       incFreeInodes(dev);

       put_block(dev, imap, &buf);


}

int incFreeBlocks(int dev)
{
    char buf[BLKSIZE];
    SUPER *sp;
    GD *gp;
    // dec free inodes count in SUPER and GD
    get_block(dev, 1, &buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, &buf);

    get_block(dev, 2, &buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, &buf);
}

int bdealloc(int devs, int blk)
{

    //printf("In balloc dev = %d\n", devs);
    char bufz[BLKSIZE];
    get_block(devs, 2, &bufz);
    GD *gp = (GD *)bufz;
    int imap = gp->bg_block_bitmap;

    // read inode_bitmap block
    get_block(devs, imap, &bufz);
    //printf("from bdealloc the blk=%d\n");
         clr_bit(bufz,blk);

         incFreeBlocks(devs);

         put_block(devs, imap, &bufz);

    //printf("In balloc dev = %d\n", devs);
/*    char bufz[BLKSIZE];

    // read inode_bitmap block
    get_block(devs, blk, &bufz);

         clr_bit(bufz,blk);
         incFreeBlocks(devs);

         put_block(devs, blk, &bufz);
*/
}

