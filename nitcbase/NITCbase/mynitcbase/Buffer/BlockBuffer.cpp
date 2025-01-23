#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

// Disk disk_run;

BlockBuffer::BlockBuffer(int blockNum)
{
    this->blockNum = blockNum;
}

RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {

}

// the declarations for these functions can be found in "BlockBuffer.h"


/*
Used to get the header of the block into the location pointed to by `head`
NOTE: this function expects the caller to allocate memory for `head`
*/
int BlockBuffer::getHeader(struct HeadInfo *head) {

  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;   // return any errors that might have occured in the process
  }

  // ... (the rest of the logic is as in stage 2)

    memcpy(&head->pblock, bufferPtr + 4, 4);
    memcpy(&head->lblock, bufferPtr + 8, 4);
    memcpy(&head->rblock, bufferPtr + 12, 4);
    memcpy(&head->numEntries, bufferPtr + 16, 4);
    memcpy(&head->numAttrs, bufferPtr + 20, 4);
    memcpy(&head->numSlots, bufferPtr + 24, 4);
  // ... (the rest of the logic is as in stage 2)

  return SUCCESS;
}

/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  // ...
    struct HeadInfo head;
    this->getHeader(&head);
    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;
    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS) {
        return ret;
    }
    
  // ... (the rest of the logic is as in stage 2
    int recordSize = attrCount * ATTR_SIZE;
    int slotMapSize = slotCount;
    unsigned char *slotPointer = bufferPtr + (HEADER_SIZE + slotMapSize + (recordSize * slotNum));
    memcpy(rec,slotPointer,recordSize);
    return SUCCESS;
}

// int RecBuffer::setRecord(union Attribute *rec, int slotNum)
// {
//     struct HeadInfo head;

//     // get the header using this.getHeader() function
//     this->getHeader(&head);
//     int attrCount = head.numAttrs;
//     int slotCount = head.numSlots;
//     unsigned char buffer1[BLOCK_SIZE];
//     Disk::readBlock(buffer1, this->blockNum);
//     // read the block at this.blockNum into a buffer

//     /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
//        - each record will have size attrCount * ATTR_SIZE
//        - slotMap will be of size slotCount
//     */
//     int recordSize = attrCount * ATTR_SIZE;
//     int slotMapSize = slotCount;
//     unsigned char *slotPointer = buffer1 + HEADER_SIZE + slotMapSize + (recordSize * slotNum); /*calculate buffer + offset */
//     ;

//     // load the record into the rec data structure
//     memcpy(slotPointer, rec, recordSize);
//     Disk::writeBlock(buffer1,this->blockNum);
//     return SUCCESS;
// }

/*
Used to load a block to the buffer and get a pointer to it.
NOTE: this function expects the caller to allocate memory for the argument
*/
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
  // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

  if (bufferNum == E_BLOCKNOTINBUFFER) {
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }

    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }

  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];

  return SUCCESS;
}

/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  // get the header of the block using getHeader() function
  getHeader(&head);

  int slotCount = head.numSlots;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  memcpy(slotMap, slotMapInBuffer, slotCount);
  return SUCCESS;
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    double diff;
    // if attrType == STRING
    //     diff = strcmp(attr1.sval, attr2.sval)
    if (attrType == STRING)
        diff = strcmp(attr1.sVal, attr2.sVal);

    // else
    //     diff = attr1.nval - attr2.nval
    else
        diff = attr1.nVal - attr2.nVal;

    /*
    if diff > 0 then return 1
    if diff < 0 then return -1
    if diff = 0 then return 0
    */
   if (diff > 0 ) return 1;
  else if (diff < 0) return -1;
    else return 0;
    
}

