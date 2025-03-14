#include "BlockBuffer.h"
#include<iostream>

#include <cstdlib>
#include <cstring>

int compareAttrs(union Attribute attr1,union Attribute attr2,int attrType){
    double diff;

    if(attrType==STRING){
        diff=strcmp(attr1.sVal,attr2.sVal);
    }
    else{
        diff=attr1.nVal-attr2.nVal;
    }

    if(diff>0) return 1;
    else if(diff<0) return -1;
    else return 0;
}


BlockBuffer::BlockBuffer(int blockNum)
{
    // initialise this.blockNum with the argument
    this->blockNum = blockNum;
}

BlockBuffer::BlockBuffer(char blockType){
    // allocate a block on the disk and a buffer in memory to hold the new block of
    // given type using getFreeBlock function and get the return error codes if any.
    int blockTy= blockType=='R'? REC:UNUSED_BLK;
    
    int blkNum=getFreeBlock(blockTy);
    if(blkNum<0||blkNum>=BLOCK_SIZE){
        this->blockNum=blkNum;
        return ;
    }

    // set the blockNum field of the object to that of the allocated block
    // number if the method returned a valid block number,
    // otherwise set the error code returned as the block number.
    this->blockNum=blkNum;
    // (The caller must check if the constructor allocatted block successfully
    // by checking the value of block number field.)
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum){}

// call parent non-default constructor with 'R' denoting record block.
RecBuffer::RecBuffer() : BlockBuffer('R'){}

/*
Used to get the header of the block into the location pointed to by `head`
NOTE: this function expects the caller to allocate memory for `head`
*/
int BlockBuffer::getHeader(struct HeadInfo *head)
{

    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS)
    {
        return ret; // return any errors that might have occured in the process
    }

    memcpy(&head->blockType,bufferPtr,4);
    memcpy(&head->pblock, bufferPtr + 4, 4);
    memcpy(&head->lblock, bufferPtr + 8, 4);
    memcpy(&head->rblock, bufferPtr + 12, 4);
    memcpy(&head->numEntries, bufferPtr + 16, 4);
    memcpy(&head->numAttrs, bufferPtr + 20, 4);
    memcpy(&head->numSlots, bufferPtr + 24, 4);

   
    
    return SUCCESS;
}

int BlockBuffer::setHeader(struct HeadInfo *head){

    unsigned char *bufferPtr;
    // get the starting address of the buffer containing the block using
    // loadBlockAndGetBufferPtr(&bufferPtr).
    int check=loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if(check!=SUCCESS){
        return check;
    }

    // cast bufferPtr to type HeadInfo*
    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

    // copy the fields of the HeadInfo pointed to by head (except reserved) to
    // the header of the block (pointed to by bufferHeader)
    //(hint: bufferHeader->numSlots = head->numSlots )
    bufferHeader->pblock=head->pblock;
    bufferHeader->lblock=head->lblock;
    bufferHeader->rblock=head->rblock;
    bufferHeader->numEntries=head->numEntries;
    bufferHeader->numAttrs=head->numAttrs;
    bufferHeader->numSlots=head->numSlots;
    bufferHeader->blockType=head->blockType;


    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed, return the error code
    int ret=StaticBuffer::setDirtyBit(this->blockNum);
    if(ret!=SUCCESS){
        printf("Error in SetDirtyBit\n");
        return ret;
    }

    return SUCCESS;

    // return SUCCESS;
}


int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char ** buffPtr) {
    /* check whether the block is already present in the buffer
       using StaticBuffer.getBufferNum() */
    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    // if present (!=E_BLOCKNOTINBUFFER),
        // set the timestamp of the corresponding buffer to 0 and increment the
        // timestamps of all other occupied buffers in BufferMetaInfo.
    if(bufferNum!=E_BLOCKNOTINBUFFER){
        
        for(int i=0;i<BUFFER_CAPACITY;i++){
            if(StaticBuffer::metainfo[i].free==false){
                StaticBuffer::metainfo[i].timeStamp+=1;
            }
        }
        StaticBuffer::metainfo[bufferNum].timeStamp=0;
    }
    else{
        bufferNum=StaticBuffer::getFreeBuffer(this->blockNum);
        if(bufferNum==E_OUTOFBOUND){
            return E_OUTOFBOUND;
        }
        Disk::readBlock(StaticBuffer::blocks[bufferNum],this->blockNum);

    }

    *buffPtr=StaticBuffer::blocks[bufferNum];

    return SUCCESS;

    // else
        // get a free buffer using StaticBuffer.getFreeBuffer()

        // if the call returns E_OUTOFBOUND, return E_OUTOFBOUND here as
        // the blockNum is invalid

        // Read the block into the free buffer using readBlock()
    

    // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr

    // return SUCCESS;
}

int RecBuffer::getSlotMap(unsigned char *SlotMap){
    unsigned char *bufferptr;

    int ret=loadBlockAndGetBufferPtr(&bufferptr);
    if(ret!=SUCCESS){
        return ret;
    }

    struct HeadInfo head;
    getHeader(&head);

    int slotCount=head.numSlots;

    unsigned char *SlotMapInBuffer=bufferptr+HEADER_SIZE;

    memcpy(SlotMap,SlotMapInBuffer,slotCount);
    
    return SUCCESS;
}

int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);
    if(ret!=SUCCESS){
        return ret;
    }

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    // get the header of the block using the getHeader() function
    HeadInfo head;
    getHeader(&head);

    int slots = head.numSlots;

    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`
   
    memcpy(bufferPtr+HEADER_SIZE,slotMap,slots);

    // update dirty bit using StaticBuffer::setDirtyBit
    // if setDirtyBit failed, return the value returned by the call
    int back=StaticBuffer::setDirtyBit(this->blockNum);
    if(back!=SUCCESS){
        printf("wrong with setDirtyBit function");
        return back;
    }

    return SUCCESS;
    // return SUCCESS
}

/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
int RecBuffer::getRecord(union Attribute *rec, int slotNum)
{
    
    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS)
    {
        return ret;
    }

    struct HeadInfo head;
    ret = BlockBuffer::getHeader(&head);
    if (ret != SUCCESS)
        return ret;

    int numAttr=head.numAttrs;
    int slotCount=head.numSlots;
    
    int recSize=numAttr*ATTR_SIZE;
    unsigned char *slotPointer= bufferPtr + (HEADER_SIZE + slotCount + (recSize*slotNum));

    memcpy(rec,slotPointer,recSize);

    return SUCCESS;
}


int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if(ret!=SUCCESS){
        return ret;
    }

    /* get the header of the block using the getHeader() function */
    HeadInfo head;
    ret = BlockBuffer::getHeader(&head);
    if (ret != SUCCESS)
        return ret;

    int numatr=head.numAttrs;
    int numslot=head.numSlots;
    if(slotNum>=numslot||slotNum<0){
        return E_OUTOFBOUND;
    }

    // get number of attributes in the block.

    // get the number of slots in the block.

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.

    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */

    int recSize=numatr*ATTR_SIZE;
    unsigned char *slotPointer= bufferPtr + (HEADER_SIZE + numslot + (recSize*slotNum));

    memcpy(slotPointer,rec,recSize);
    int back=StaticBuffer::setDirtyBit(this->blockNum);
    if(back!=SUCCESS){
        printf("wrong with setDirtyBit function");
        return back;
    }

    return SUCCESS;

    // update dirty bit using setDirtyBit()

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */

    // return SUCCESS
}


int BlockBuffer::setBlockType(int blockType){

    
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    // store the input block type in the first 4 bytes of the buffer.
    // (hint: cast bufferPtr to int32_t* and then assign it)
    // *((int32_t *)bufferPtr) = blockType;

    // update the StaticBuffer::blockAllocMap entry corresponding to the
    // object's block number to `blockType`.

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed
        // return the returned value from the call

    unsigned char *bufferPtr;
    int check=loadBlockAndGetBufferPtr(&bufferPtr);
    if(check!=SUCCESS){
        return check;
    }

    *((int32_t*)bufferPtr)=blockType;
    StaticBuffer::blockAllocMap[this->blockNum]=blockType;

    check=StaticBuffer::setDirtyBit(this->blockNum);
    if(check!=SUCCESS){
        printf("wrong with setDirtyBit function");
        return check;
    }

    return SUCCESS;

    // return SUCCESS
}

int BlockBuffer::getFreeBlock(int blockType){

    // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
    // if no block is free, return E_DISKFULL.

    int i=0;
    for(;i<DISK_BLOCKS;i++){
        if(StaticBuffer::blockAllocMap[i]==UNUSED_BLK){
            break;
        }
    }
    if(i==DISK_BLOCKS){
        return E_DISKFULL;
    }


    // set the object's blockNum to the block number of the free block.
    this->blockNum=i;

    // find a free buffer using StaticBuffer::getFreeBuffer() .
    int buffNum=StaticBuffer::getFreeBuffer(i);
    if(buffNum<0||buffNum>=BUFFER_CAPACITY){
        printf("Error in FreeBuffer\n");
        return buffNum;
    }

    // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    // to the setHeader() function.

    // update the block type of the block to the input block type using setBlockType().
    struct HeadInfo head;
    head.pblock=-1;
    head.lblock=-1;
    head.rblock=-1;
    head.numEntries=0;
    head.numAttrs=0;
    head.numSlots=0;
    head.blockType=blockType;
    int check=setHeader(&head);
    if(check!=SUCCESS){
        return check;
    }

    int ret =setBlockType(blockType);
    if(ret!=SUCCESS){
        return ret;
    }

    return i;
    // return block number of the free block.
}

int BlockBuffer::getBlockNum(){
    return this->blockNum;
    // return corresponding block number.
}

void BlockBuffer::releaseBlock(){

    // if blockNum is INVALID_BLOCKNUM (-1), or it is invalidated already, do nothing
    if(this->blockNum==INVALID_BLOCKNUM||StaticBuffer::blockAllocMap[this->blockNum]==UNUSED_BLK){
        return;
    }

    int buffNum=StaticBuffer::getBufferNum(this->blockNum);
    if(buffNum!=E_BLOCKNOTINBUFFER){
        StaticBuffer::metainfo[buffNum].free=true;
        
    }
    StaticBuffer::blockAllocMap[this->blockNum]=UNUSED_BLK;
    this->blockNum=INVALID_BLOCKNUM;
    // else
        /* get the buffer number of the buffer assigned to the block
           using StaticBuffer::getBufferNum().
           (this function return E_BLOCKNOTINBUFFER if the block is not
           currently loaded in the buffer)
            */

        // if the block is present in the buffer, free the buffer
        // by setting the free flag of its StaticBuffer::tableMetaInfo entry
        // to true.

        // free the block in disk by setting the data type of the entry
        // corresponding to the block number in StaticBuffer::blockAllocMap
        // to UNUSED_BLK.

        // set the object's blockNum to INVALID_BLOCK (-1)
}


// !creating index from here onwards
// call the corresponding parent constructor
IndBuffer::IndBuffer(char blockType) : BlockBuffer(blockType) {}

// call the corresponding parent constructor
IndBuffer::IndBuffer(int blockNum) : BlockBuffer(blockNum) {}

IndInternal::IndInternal() : IndBuffer('I') {}
// call the corresponding parent constructor
// 'I' used to denote IndInternal.

IndInternal::IndInternal(int blockNum) : IndBuffer(blockNum) {}
// call the corresponding parent constructor

IndLeaf::IndLeaf()
    : IndBuffer('L') {} // this is the way to call parent non-default
                        // constructor. 'L' used to denote IndLeaf.

// this is the way to call parent non-default constructor.
IndLeaf::IndLeaf(int blockNum) : IndBuffer(blockNum) {}
//!get entry of Indleaf
int IndLeaf::getEntry(void *ptr, int indexNum) {

  // if the indexNum is not in the valid range of [0, MAX_KEYS_LEAF-1]
  //     return E_OUTOFBOUND.
  if (indexNum < 0 or indexNum >= MAX_KEYS_LEAF) {
    return E_OUTOFBOUND;
  }

  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  //     return the value returned by the call.

  // copy the indexNum'th Index entry in buffer to memory ptr using memcpy

  /* the indexNum'th entry will begin at an offset of
     HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
  unsigned char *entryPtr =bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
  memcpy((struct Index *)ptr, entryPtr, LEAF_ENTRY_SIZE);
  return SUCCESS;
  // return SUCCESS
}

//! indInternal getEntry
int IndInternal::getEntry(void *ptr, int indexNum) {
  // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
  //     return E_OUTOFBOUND.
  if (indexNum < 0 or indexNum >= MAX_KEYS_LEAF) {
    return E_OUTOFBOUND;
  }

  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS)
    return ret;

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  //     return the value returned by the call.

  // typecast the void pointer to an internal entry pointer
  struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;

  /*
  - copy the entries from the indexNum`th entry to *internalEntry
  - make sure that each field is copied individually as in the following code
  - the lChild and rChild fields of InternalEntry are of type int32_t
  - int32_t is a type of int that is guaranteed to be 4 bytes across every
    C++ implementation. sizeof(int32_t) = 4
  */

  /* the indexNum'th entry will begin at an offset of
     HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
     from bufferPtr */
  unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);

  memcpy(&(internalEntry->lChild), entryPtr, sizeof(int32_t));
  memcpy(&(internalEntry->attrVal), entryPtr + 4, sizeof(Attribute));
  memcpy(&(internalEntry->rChild), entryPtr + 20, 4);

  return SUCCESS;

  // return SUCCESS.
}
//todo::Add the following empty definitions to avoid compilation issues. We will implement 
//todo these functions in later stages.

int IndInternal::setEntry(void *ptr, int indexNum) {
  return 0;
}

int IndLeaf::setEntry(void *ptr, int indexNum) {
  return 0;
}