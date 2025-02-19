#include "StaticBuffer.h"


unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {

  // initialise all blocks as free
  for (int i=0;i<BUFFER_CAPACITY;i++) {
    metainfo[i].free = true;
    metainfo[i].dirty=false;
    metainfo[i].timeStamp=-1;
    metainfo[i].blockNum=-1;
  }
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/
StaticBuffer::~StaticBuffer() {

  for(int i=0;i<BUFFER_CAPACITY;i++){
    if(metainfo[i].dirty==true&&metainfo[i].free==false){
      Disk::writeBlock(blocks[i],metainfo[i].blockNum);
    }
  }
}

int StaticBuffer::getFreeBuffer(int blockNum) {
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  for(int i=0;i<BUFFER_CAPACITY;i++){
    if(metainfo[i].free==false){
        metainfo[i].timeStamp+=1;
    }
  }

  int buffNum;

  // iterate through metainfo and check if there is any buffer free

    // if a free buffer is available, set bufferNum = index of that free buffer.
  int i=0;
  for(;i<BUFFER_CAPACITY;i++){
    if(metainfo[i].free==true){
        buffNum=i;
        break;
    }
  }
  if(i==BUFFER_CAPACITY){
    int maxb,maxt=-1;

    for(int j=0;j<BUFFER_CAPACITY;j++){
        if(metainfo[j].timeStamp>maxt){
          maxb=j;
          maxt=metainfo[i].timeStamp;
        }
    }
    if(metainfo[maxb].dirty==true){
        Disk::writeBlock(blocks[maxb],metainfo[maxb].blockNum);
    }
    buffNum=maxb;
  }

  metainfo[buffNum].free = false;
  metainfo[buffNum].dirty=false;
  metainfo[buffNum].timeStamp=0;
  metainfo[buffNum].blockNum=blockNum;

  return buffNum;

    // if a free buffer is not available,
    //     find the buffer with the largest timestamp
    //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    //     set bufferNum = index of this buffer

    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.

    // return the bufferNum.
}



/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {

  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  for(int i=0;i<BUFFER_CAPACITY;i++){
    if(metainfo[i].blockNum==blockNum){
        return i;
    }
  }

  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}


int StaticBuffer::setDirtyBit(int blockNum){
  // find the buffer index corresponding to the block using getBufferNum().
  // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
  //     return E_BLOCKNOTINBUFFER
  int bufferInd=getBufferNum(blockNum);
  if(bufferInd==E_BLOCKNOTINBUFFER){
    return E_BLOCKNOTINBUFFER;
  }

  // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
  //     return E_OUTOFBOUND
  
  // else
  //     (the bufferNum is valid)
  //     set the dirty bit of that buffer to true in metainfo
  if(bufferInd==E_OUTOFBOUND){
    return E_OUTOFBOUND;
  }
  metainfo[bufferInd].dirty=true;

  return SUCCESS;

  // return SUCCESS
}
