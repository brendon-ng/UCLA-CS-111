// NAME: Brendon Ng
// EMAIL: brendonn8@gmail.com
// ID: 304925492


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "ext2_fs.h"

struct ext2_super_block superBlock;
struct ext2_group_desc groupDesc;
__u32 blocksize;

void free_entries(int blockOrInode, int image){
  __u32 offset;
  char type;
  __u32 maxBytes;

  if(blockOrInode == 0){
    type = 'B';
    offset = groupDesc.bg_block_bitmap * EXT2_MIN_BLOCK_SIZE;
    maxBytes = ceil(superBlock.s_blocks_count/8);
  }
  else{
    type = 'I';
    offset = groupDesc.bg_inode_bitmap * EXT2_MIN_BLOCK_SIZE;
    maxBytes = ceil(superBlock.s_inodes_count/8);
  }

  for(__u32 i = 0; i < maxBytes; i++){
    __u8 cur;
    size_t count = sizeof(__u8);
    if(pread(image, &cur, count, offset+i) < (ssize_t)count){
      fprintf(stderr, "Error: pread() failed.\n");
      fflush(stderr);
      exit(2);
    }
    int mask = 1;
    for(int j=0; j<8; j++){
      if((mask & cur) == 0){
	printf("%cFREE,%u\n", type,(i*8)+(j+1));
      }
      mask <<= 1 ;
    }
  }

}

void indirect(__u32 b, __u32 i, __u32 offset, int level, char type, struct ext2_inode* cur, int image){
  __u32 block[blocksize];

  if(pread(image, &block, blocksize, b*blocksize) < (ssize_t)blocksize){
    fprintf(stderr, "Error: pread() failed.\n");
    fflush(stderr);
    exit(2);
  }
  __u32 numEntries = blocksize/4;
  for(__u32 j=0; j<numEntries; j++){
    if(block[j] != 0){
      if(type == 'd' && level == 1){
	unsigned char temp[blocksize];
	unsigned int tempoffset = 0;

	if(pread(image, &temp, blocksize, b*blocksize) < (ssize_t)blocksize){
	  fprintf(stderr, "Error: pread() failed.\n");
	  fflush(stderr);
	  exit(2);
	}
	
	struct ext2_dir_entry* curDirEnt = (struct ext2_dir_entry*) temp;
	while(tempoffset < cur->i_size && curDirEnt->file_type){
	  if(curDirEnt->inode !=0){
	    char file[EXT2_NAME_LEN +1];
	    memcpy(file, curDirEnt->name, curDirEnt->name_len);
	    file[curDirEnt->name_len] = '\0';
	    printf("DIRENT,%d,%u,%u,%u,%u,'%s'\n",i,(offset*blocksize)+tempoffset,curDirEnt->inode,curDirEnt->rec_len,curDirEnt->name_len,file);
	  }
	  tempoffset+= curDirEnt->rec_len;
	  curDirEnt = (void*) curDirEnt + curDirEnt->rec_len;
	}
      }

      printf("INDIRECT,%d,%u,%u,%u,%u\n",i,level,offset,b,block[j]);

      if(level >1){
	indirect(block[j], i, offset, level-1, type, cur, image);
	if(level == 2)
	  offset += 256;
	else if(level ==3)
	  offset += 65536;
      }
    }
    if(level ==1)
      offset +=1;
  }
}


int main(int argc, char* argv[]){
  if(argc != 2){
    fprintf(stderr, "Error: Invalid usage. Usage: ./lab3a fileSystem.img\n");
    fflush(stderr);
    exit(1);
  }

  int image = open(argv[1], O_RDONLY);
  if(image < 0){
    fprintf(stderr, "Error opening image\n");
    fflush(stderr);
    exit(1);
  }

  //Super Block
  size_t count = sizeof(struct ext2_super_block);
  if(pread(image, &superBlock, count, 1024) < (ssize_t)count){
    fprintf(stderr, "Error: pread() failed.\n");
    fflush(stderr);
    exit(2);
  }

  blocksize = EXT2_MIN_BLOCK_SIZE << superBlock.s_log_block_size;

  printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", superBlock.s_blocks_count,superBlock.s_inodes_count,blocksize,superBlock.s_inode_size,superBlock.s_blocks_per_group,superBlock.s_inodes_per_group,superBlock.s_first_ino);


  //Group
  count = sizeof(struct ext2_group_desc);
  if(pread(image, &groupDesc, count, 2048) < (ssize_t) count){
    fprintf(stderr, "Error: pread() failed.\n");
    fflush(stderr);
    exit(2);
  }
  int groupnum = 0;
  printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n", groupnum,superBlock.s_blocks_count,superBlock.s_inodes_count,groupDesc.bg_free_blocks_count,groupDesc.bg_free_inodes_count,groupDesc.bg_block_bitmap,groupDesc.bg_inode_bitmap,groupDesc.bg_inode_table);

  free_entries(0, image); //Free block entries
  free_entries(1, image); //Free inode entries

  //I-node
  __u32 inodeOffset = groupDesc.bg_inode_table * EXT2_MIN_BLOCK_SIZE;

  for(__u32 i=1; i<=superBlock.s_inodes_count; i++){
    struct ext2_inode cur;
    count = superBlock.s_inode_size;
    if(pread(image, &cur, count, inodeOffset+(i-1)* superBlock.s_inode_size) < (ssize_t)count){
      fprintf(stderr, "Error: pread() failed.\n");
      fflush(stderr);
      exit(2);
    }

    if(cur.i_mode == 0 || cur.i_links_count == 0)
      continue;

    char type;
    if(cur.i_mode & 0x8000)
      type = 'f';
    else if(cur.i_mode & 0x4000)
      type = 'd';
    else if(cur.i_mode & 0xA000)
      type = 's';
    else
      type = '?';

    char ctime[20];
    char mtime[20];
    char atime[20];
    time_t raw = cur.i_ctime;
    struct tm times = *(gmtime(&raw));
    strftime(ctime, 20, "%m/%d/%y %H:%M:%S", &times);
    raw = cur.i_mtime;
    times = *(gmtime(&raw));
    strftime(mtime, 20, "%m/%d/%y %H:%M:%S", &times);
    raw = cur.i_atime;
    times = *(gmtime(&raw));
    strftime(atime, 20, "%m/%d/%y %H:%M:%S", &times);

    printf("INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u", i,type,cur.i_mode&0x0FFF,cur.i_uid,cur.i_gid,cur.i_links_count,ctime,mtime,atime,cur.i_size,cur.i_blocks);

    if(type=='f'||type=='d'||(type=='s' && cur.i_size> 60)){
      for(int j=0; j<EXT2_N_BLOCKS; j++)
	printf(",%u",cur.i_block[j]);
    }
    printf("\n");

    if(type == 'd'){
      unsigned char block[blocksize];
      struct ext2_dir_entry* curDirEnt;
      unsigned int diroffset = 0;
      for(int j=0; j<EXT2_NDIR_BLOCKS; j++){
	if(pread(image, block, blocksize, cur.i_block[j]*blocksize) < (ssize_t)blocksize){
	  fprintf(stderr, "Error: pread() failed.\n");
	  fflush(stderr);
	  exit(2);
	}
	curDirEnt = (struct ext2_dir_entry*) block;

	while(diroffset < cur.i_size && curDirEnt->file_type){
	  if(curDirEnt->inode != 0){
	    char file[EXT2_NAME_LEN +1];
	    memcpy(file, curDirEnt->name, curDirEnt->name_len);
	    file[curDirEnt->name_len] = '\0';

	    printf("DIRENT,%d,%u,%u,%u,%u,'%s'\n", i, diroffset,curDirEnt->inode,curDirEnt->rec_len,curDirEnt->name_len,file);
	  }
	  diroffset += curDirEnt->rec_len;
	  curDirEnt = (void*) curDirEnt + curDirEnt->rec_len;
	}
      }
    }
    
    if(type =='d' || type == 'f'){
      if(cur.i_block[12]!=0)
	indirect(cur.i_block[12], i, 12, 1, type, &cur, image);
      if(cur.i_block[13]!=0)
	indirect(cur.i_block[13], i, 12+256, 2, type, &cur, image);
      if(cur.i_block[14]!=0)
        indirect(cur.i_block[14], i, 12+256+256*256, 3, type, &cur, image);
    }
  }
  
}
