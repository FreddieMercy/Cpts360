/********* super.c code ***************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
//#include <ext2fs/ext2_fs.h>
#include "ext2_fs.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netdb.h>


#include <sys/types.h>

#include <stdarg.h>

typedef unsigned char  u8;            // unsigned char
typedef unsigned short u16;           // unsigned short
typedef unsigned int   u32;           // unsigned int

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

#define BLKSIZE 1024

char buf[BLKSIZE];
int fd;

int get_block(int fds, int blk, char bufs[ ])
{
  lseek(fds, (long)blk*BLKSIZE, 0);
  read(fds, bufs, BLKSIZE);
}

int get_inode(int fds, int blk, char bufs[ ])
{
  lseek(fds, (blk-1)* sizeof(struct ext2_inode)+gp->bg_inode_table*BLKSIZE, 0);
  read(fds, bufs, sizeof(struct ext2_inode));
}

gd()
{
printf("\n\n************  group 0 info: *************\n\n");
  // read SUPER block
  get_block(fd, 2, buf);  
  gp = (GD *)buf;

  printf("bg_block_bitmap = %d\n", gp->bg_block_bitmap);
  printf("bg_inode_bitmap = %d\n", gp->bg_inode_bitmap);

  printf("bg_inode_table = %d\n", gp->bg_inode_table);
  printf("bg_free_blocks_count = %d\n", gp->bg_free_blocks_count);
  printf("bg_free_inodes_count = %d\n", gp->bg_free_inodes_count);
  printf("bg_used_dirs_count = %d\n", gp->bg_used_dirs_count);
}


inode()
{
printf("\n\n************  root inode info: *************\n\n");

u32 iblock = gp->bg_inode_table;   // get inode start block#
  printf("inode_block=%d\n", iblock);

  // get inode start block     
  get_block(fd, iblock, buf);

  ip = (INODE *)buf + 1;         // ip points at 2nd INODE
  
  printf("mode=%4x ", ip->i_mode);
  printf("uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
  printf("size=%d\n", ip->i_size);
  printf("time=%s\n", ctime(&ip->i_ctime));
  printf("link=%d\n", ip->i_links_count);
  //printf("i_block[0]=%d\n", ip->i_block[0]);

}

super()
{
printf("\n\n************  super block info: *************\n\n");
  // read SUPER block
  get_block(fd, 1, buf);  
  sp = (SUPER *)buf;

  // check for EXT2 magic number:

  printf("s_magic = %x\n", sp->s_magic);
  if (sp->s_magic != 0xEF53){
    printf("NOT an EXT2 FS\n");
    exit(1);
  }

  printf("EXT2 FS OK\n");

  printf("s_inodes_count = %d\n", sp->s_inodes_count);
  printf("s_blocks_count = %d\n", sp->s_blocks_count);

  printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
  printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);
  printf("s_first_data_blcok = %d\n", sp->s_first_data_block);


  printf("s_log_block_size = %d\n", sp->s_log_block_size);
  printf("s_log_frag_size = %d\n", sp->s_log_frag_size);

  printf("s_blocks_per_group = %d\n", sp->s_blocks_per_group);
  printf("s_frags_per_group = %d\n", sp->s_frags_per_group);
  printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);

  printf("s_mnt_count = %d\n", sp->s_mnt_count);
  printf("s_max_mnt_count = %d\n", sp->s_max_mnt_count);

  printf("s_magic = %x\n", sp->s_magic);

  printf("s_mtime = %s\n", ctime(&sp->s_mtime));
  printf("s_wtime = %s\n", ctime(&sp->s_wtime));

printf("---------------------------------------------\n");
printf("inodes_per_block = %d\n\n\n", BLKSIZE / sizeof(struct ext2_inode));

}


u32 search(INODE *io, char *name)
{/*
            search for name string in DIR's data block(s);
            if FOUND: return its ino;
            return 0;  // for NOT FOUND*/


		printf("i_block[0] = %d\n\n", io->i_block[0]); // print blk number
		printf("i_number  rec_len  name_len  name\n");
		get_block(fd, io->i_block[0], buf);     // read INODE's i_block[0]

		char *cp;  
		char temp[256];
		int found=0;

		cp = buf;  
		dp = (DIR *)buf;

		DIR *tmp = NULL;
		
		while(cp < buf + BLKSIZE){

		     strncpy(temp, dp->name, dp->name_len);
		     temp[dp->name_len] = 0;

			if(strcmp(name, temp)==0)
			{
				found=1;
				tmp = dp;
			}

		     printf("%d  \t  %d  \t   %d  \t     %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
	     
		     // move to the next DIR entry:
		     cp += dp->rec_len;   // advance cp by rec_len BYTEs
		     dp = (DIR *)cp;     // pull dp along to the next record
		}

		if(found)
		{
			printf("\nFound %s : ino = %d\n", name, tmp->inode);

			if(tmp != NULL)
			{
				return tmp->inode;
			}

			return 0;

		}

		return 0;

}



char *disk = "mydisk";

main(int argc, char *argv[ ])

{ 
  if (argc > 1)
    disk = argv[1];
  fd = open(disk, O_RDONLY);
  if (fd < 0){
    printf("open failed\n");
    exit(1);
  }

  super();
  gd();
  inode();

	printf("\n\n********* root dir entries ***********\n\n");

	char *tar=NULL;

	tar = strtok(argv[2],"/");
		
	int i = 0;

	while(tar != NULL)
	{
		printf("i=%d name[%d]=%s\nsearch for %s in %x\n",i,i,tar, tar, ip->i_block[0]);
		u32 inumber = search(ip, tar);

		if(inumber == 0)
		{
			printf("can't find it, BOMB OUT!\n");
			return 0;
		}


		  get_block(fd, 2, buf);  
		  gp = (GD *)buf;

		u32 iblock = gp->bg_inode_table; 

		get_block(fd, iblock+((inumber- 1)/(BLKSIZE / sizeof(struct ext2_inode))), buf);

		ip = (INODE *)buf + (inumber- 1)%(BLKSIZE / sizeof(struct ext2_inode)); 

		

		tar=strtok(NULL,"/");

		i++;


		printf("mode: %x \t uid: %d \t size: %d \t \ngroup: %d \t links count: %d \t blocks: %d \t \n",ip->i_mode, ip->i_uid, ip->i_size, ip->i_gid, ip->i_links_count, ip->i_blocks);
/*
		int j = 0;
		printf("****************  DISK BLOCKS  *******************\n");
		for(j=0; j < 13; j++)
		{
			printf("block[\t%d] = %d\n", j, ip->i_block[j]);
		}

		printf("================ Direct Blocks ===================\n");
		printf("%d\n", ip->i_block[13]);

		printf("===============  Indirect blocks   ===============\n");
		printf("%d\n", ip->i_block[14]);
*/
			printf("\n---------------------------------------------\n");

	}

	return 0;
}




