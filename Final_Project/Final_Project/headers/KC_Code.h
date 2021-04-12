#include "include.h"

mountroot()   /* mount root file system */
{
  MOUNT *mp;

  char buf[BLKSIZE];
  SUPER *sp;
  GD *gp;
  char line[64], *rootdev;
  int ninodes, nblocks, ifree, bfree;

  printf("enter rootdev name (RETURN for disk) : ");
  gets(line);

  rootdev = "disk";

  if (line[0] != 0)
     rootdev = line;

  int dev = open(rootdev, O_RDWR);
  //fd = dev;
  //printf("\n\nThe fd in init() is %d\n\n", dev);
  if (dev < 0){
     printf("panic : can't open root device\n");
     exit(1);
  }

  /* get super block of rootdev */
  get_block(dev, 1, &buf);
  sp = (SUPER *)buf;

  /* check magic number */
  printf("SUPER magic=0x%x  ", sp->s_magic);
  if (sp->s_magic != SUPER_MAGIC){
     printf("super magic=%x : %s is not a valid Ext2 filesys\n",
             sp->s_magic, rootdev);
     exit(0);
  }

  mp = &mounttab[0];      /* use mounttab[0] */

  /* copy super block info to mounttab[0] */
  ninodes = mp->ninodes = sp->s_inodes_count;
  nblocks = mp->nblocks = sp->s_blocks_count;

  bfree = sp->s_free_blocks_count;
  ifree = sp->s_free_inodes_count;

  get_block(dev, 2, &buf);
  gp = (GD *)buf;

  mp->dev = dev;
  mp->busy = BUSY;

  mp->bmap = gp->bg_block_bitmap;
  mp->imap = gp->bg_inode_bitmap;
  mp->iblock = gp->bg_inode_table;

  strcpy(mp->name, rootdev);
  strcpy(mp->mount_name, "/");


  printf("bmap=%d  ",   gp->bg_block_bitmap);
  printf("imap=%d  ",   gp->bg_inode_bitmap);
  printf("iblock=%d\n", gp->bg_inode_table);

  /***** call iget(), which inc the Minode's refCount ****/

  root = iget(dev, 2);          /* get root inode */

  mp->mounted_inode = root;
  root->mountptr = mp;

  printf("mount : %s  mounted on / \n", rootdev);
  printf("nblocks=%d  bfree=%d   ninodes=%d  ifree=%d\n",
      nblocks, bfree, ninodes, ifree);


  return(0);
}

init()
{
  int i, j;
  PROC *p;

  //MINODES = 100? 50?
  for (i=0; i<NMINODES; i++)
      minode[i].refCount = 0;

  for (i=0; i<NMOUNT; i++)
      mounttab[i].busy = 0;

  for (i=0; i<NPROC; i++){
      proc[i].status = FREE;
      for (j=0; j<NFD; j++)
          proc[i].fd[j] = 0;
      proc[i].next = &proc[i+1];
  }

  for (i=0; i<NOFT; i++)
      oft[i].refCount = 0;

  printf("mounting root\n");
    mountroot();
  printf("mounted root\n");

  printf("creating P0, P1\n");
  p = running = &proc[0];
  p->status = BUSY;
  p->uid = 0;
  p->pid = p->ppid = p->gid = 0;
  p->parent = p->sibling = p;
  p->child = 0;
  p->cwd = root;
  p->cwd->refCount++;

  p = &proc[1];
  p->next = &proc[0];
  p->status = BUSY;
  p->uid = 2;
  p->pid = 1;
  p->ppid = p->gid = 0;
  p->cwd = root;
  p->cwd->refCount++;

  nproc = 2;


}
