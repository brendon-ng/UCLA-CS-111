#!/usr/bin/python

import sys
import csv
from collections import defaultdict

superBlock = None
bfree = []
ifree = []
inodes = []
dirents = []
indirects = []
exit_stat = 0
refs_to_blocks = defaultdict(list)
levels = ["", "INDIRECT ", "DOUBLE INDIRECT ", "TRIPLE INDIRECT "]

class SuperBlock:
   def __init__(self, row):
      self.n_blocks = int(row[1])
      self.n_inodes = int(row[2])
      self.blocks= int(row[5])
      self.inodes= int(row[6])
      self.firstInode =int(row[7])
      self.firstBlock = self.n_inodes*int(row[4]) / int(row[3]) + 1 + 4

class Inode:
   def __init__(self, row):
      self.num = int(row[1])
      self.fileType = row[2]
      self.links = int(row[6])
      self.s_file = int(row[10])
      self.directBlocks = map(int, row[12:24])
      self.singleIndirect = int(row[24])
      self.doubleIndirect = int(row[25])
      self.tripleIndirect = int(row[26])
class Dirent:
   def __init__(self,row):
      self.parent = int(row[1])
      self.offset = int(row[2])
      self.inode = int(row[3])
      self.name = row[6]

class Indirect:
   def __init__(self, row):
      self.inode = int(row[1])
      self.level = int(row[2])
      self.offset = int(row[3])
      self.indirectBlock = int(row[4])
      self.referenceBlock = int(row[5])

class Reference:
   def __init__(self, block, inode, offset, level):
      self.block = block
      self.inode = inode
      self.offset = offset
      self.level = level

def validBlock(block, inode, offset, level):
   global exit_stat
   if level == 0:
      type = ""
   elif level == 1:
      type = "INDIRECT "
   elif level == 2:
      type = "DOUBLE INDIRECT "
   elif level == 3:
      type = "TRIPLE INDIRECT "
   
   if block == 0:
      return

   # check for invalidity
   if block < 0 or block > superBlock.n_blocks:
      print("INVALID "+ str(type) + "BLOCK " + str(block) + " IN INODE " + str(inode) + " AT OFFSET " + str(offset))
      exit_stat = 2
   elif block < superBlock.firstBlock:
      print("RESERVED " + str(type) + "BLOCK " + str(block) + " IN INODE " + str(inode) + " AT OFFSET " + str(offset))
      exit_stat = 2
   else: # valid
      refs_to_blocks[block].append(Reference(block, inode, offset, level))

if __name__ == '__main__':
   if len(sys.argv) != 2:
      sys.stderr.write("Incorrect number of arguments! Usage: ./lab3b file.csv\n")
      exit(1)
      
   csv = csv.reader(open(sys.argv[1],'r'))
   for row in csv:
      if len(row) <= 0:
         sys.stderr.write("Error: Empty line in file!\n")
         exit(1)

      type = row[0]
      if type == "SUPERBLOCK":
         superBlock = SuperBlock(row);
      elif type == "GROUP":
         continue
      elif type == "BFREE":
         bfree.append(int(row[1]))
      elif type == "IFREE":
         ifree.append(int(row[1]))
      elif type == "INODE":
         inodes.append(Inode(row))
      elif type == "DIRENT":
         dirents.append(Dirent(row))
      elif type == "INDIRECT":
         indirects.append(Indirect(row))
      else:
         sys.stderr.write("Error: Invalid entry type in csv file: %s\n" % type)
         exit(1)

   for inode in inodes:
      if inode.fileType == 's' and inode.s_file <= 60:
         continue

      #direct blocks
      offset = 0
      for b in inode.directBlocks:
         validBlock(b, inode.num, offset, 0)
         offset+=1

      #check indirect blocks
      validBlock(inode.singleIndirect, inode.num, 12, 1)
      validBlock(inode.doubleIndirect, inode.num, 268, 2)
      validBlock(inode.tripleIndirect, inode.num, 65804, 3)

   for indirect in indirects:
      validBlock(indirect.referenceBlock, indirect.inode, indirect.offset, indirect.level)

   for block in range(superBlock.firstBlock, superBlock.n_blocks):
      if block not in refs_to_blocks and block not in bfree:
         print("UNREFERENCED BLOCK " + str(block))
         exit_stat = 2
      elif block in refs_to_blocks and block in bfree:
         print("ALLOCATED BLOCK " + str(block) + " ON FREELIST")
         exit_stat = 2
      elif block in refs_to_blocks and len(refs_to_blocks[int(block)]) > 1:
         for block_ref in refs_to_blocks[int(block)]:
            print("DUPLICATE " + levels[int(block_ref.level)] + "BLOCK " + str(block_ref.block) + " IN INODE " + str(block_ref.inode) + " AT OFFSET " + str(block_ref.offset))
            exit_stat = 2

   # Inodes
   unallocated = ifree
   allocated = []
   for inode in inodes:
      if inode.fileType != '0':
         if inode.num in ifree:
            print("ALLOCATED INODE " + str(inode.num) + " ON FREELIST")
            unallocated.remove(inode.num)
            exit_stat = 2
         allocated.append(inode)
      else:
         if inode.num not in ifree:
            print("UNALLOCATED INODE " + str(inode.num) + " NOT ON FREELIST")
            unallocated.append(inode.num)
      for i in range(superBlock.firstInode, superBlock.n_inodes):
         if i not in ifree:
            cont = False
            for j_inode in inodes:
               if j_inode.num == i:
                  cont = True
            if not cont:
               print("UNALLOCATED INODE " + str(i) + " NOT ON FREELIST")
               unallocated.append(i)
               exit_stat=2

   links = {}
   parents = {2:2}
   for dirent in dirents:
      if dirent.inode > superBlock.n_inodes:
         print("DIRECTORY INODE "+str(dirent.parent)+" NAME "+str(dirent.name)+" INVALID INODE "+str(dirent.inode))
         exit_stat = 2
      elif dirent.inode in unallocated:
         print("DIRECTORY INODE "+str(dirent.parent)+" NAME "+str(dirent.name)+" UNALLOCATED INODE "+str(dirent.inode))
         exit_stat =2
      else:
         links[dirent.inode] = (links.get(dirent.inode, 0) + 1)

      if dirent.inode <= superBlock.n_inodes and dirent.inode not in unallocated:
         if dirent.name != "'.'" and dirent.name != "'..'":
            parents[dirent.inode] = dirent.parent
         elif dirent.name == "'.'" and dirent.inode != dirent.parent:
            print("DIRECTORY INODE " + str(dirent.parent)+" NAME '.' LINK TO INODE "+str(dirent.inode)+" SHOULD BE" + dirent.parent)
            exit_stat = 2

   for dirent in dirents:
      if dirent.name == "'..'" and parents[dirent.parent] != dirent.inode:
         print("DIRECTORY INODE "+str(dirent.parent)+" NAME '..' LINK TO INODE "+str(dirent.inode)+" SHOULD BE "+str(parents[dirent.parent]))
         exit_stat = 2

   for alloc in allocated:
      if links.get(alloc.num, 0) != alloc.links:
         print("INODE " + str(alloc.num)+" HAS "+str(links.get(alloc.num,0))+" LINKS BUT LINKCOUNT IS "+str(alloc.links))
         exit_stat =2

   


   
         
   exit(exit_stat)
