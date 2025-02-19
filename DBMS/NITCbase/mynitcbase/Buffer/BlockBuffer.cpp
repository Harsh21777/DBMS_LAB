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

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum)
{
}

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

    memcpy(&head->pblock, bufferPtr + 4, 4);
    memcpy(&head->lblock, bufferPtr + 8, 4);
    memcpy(&head->rblock, bufferPtr + 12, 4);
    memcpy(&head->numEntries, bufferPtr + 16, 4);
    memcpy(&head->numAttrs, bufferPtr + 20, 4);
    memcpy(&head->numSlots, bufferPtr + 24, 4);
    
    return SUCCESS;
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

/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
int RecBuffer::getRecord(union Attribute *rec, int slotNum)
{
    struct HeadInfo head;
    this->getHeader(&head);

    int numAttr=head.numAttrs;
    int slotCount=head.numSlots;

    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS)
    {
        return ret;
    }
    
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
    this->getHeader(&head);

    int numatr=head.numAttrs;
    int numslot=head.numSlots;
    if(slotNum>=numslot&&slotNum<0){
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
    }

    return SUCCESS;

    // update dirty bit using setDirtyBit()

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */

    // return SUCCESS
}
