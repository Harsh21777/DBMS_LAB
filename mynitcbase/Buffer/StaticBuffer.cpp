#include "StaticBuffer.h"

#include <cstdlib>
#include <cstring>

// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];


StaticBuffer::StaticBuffer(){
    // copy Block Allocation Map blocks from disk to blockAllocMap using Disk::readBlock()
    unsigned char buffer[BLOCK_SIZE];
    int blockAllocMapSlot=0;
    for(int bIndex=0;bIndex<DISK_BLOCKS/BLOCK_SIZE;bIndex++){
        Disk::readBlock(buffer,bIndex);
        for (int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++)
			StaticBuffer::blockAllocMap[blockAllocMapSlot] = buffer[slot];//reading each byte from buffer to blockAllocMap

    }
    

    //initialize metaInfo of all the buffer blocks with free:true, dirty:false, blockNum:-1 and timeStamp:-1.
    for(int bufferNO=0;bufferNO<BUFFER_CAPACITY;bufferNO++){
        metainfo[bufferNO].free=true;
        metainfo[bufferNO].dirty=false;
        metainfo[bufferNO].blockNum=-1;
        metainfo[bufferNO].timeStamp=-1;

    }

}

StaticBuffer::~StaticBuffer(){
    // copy blockAllocMap to Block Allocation Map blocks in the disk using Disk::writeBlock().
    
    //NOT REQ FOR STAGE -3

    // unsigned char buffer[BLOCK_SIZE];
    // int blockAllocMapSlot=0;
    // for(int bIndex=0;bIndex<DISK_BLOCKS/BLOCK_SIZE;bIndex++){
    //     for (int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++)
	// 		buffer[slot] = blockAllocMap[blockAllocMapSlot];//writing each byte from blockAllocMap to buffer to disk 
        
    //     Disk::writeBlock(buffer,bIndex);

    // }
    /* iterate through all the metaInfo entries,
      write back buffer blocks with meta-info as free:false,dirty:true using Disk::writeBlock().*/
    // for (int bufferNO=0;bufferNO<BUFFER_CAPACITY;bufferNO++){
    //     if(metainfo[bufferNO].free==false&&metainfo[bufferNO].dirty==true){
    //         Disk::writeBlock(blocks[bufferNO],metainfo[bufferNO].blockNum);    //NOT REQ  FOR STAGE -3   
    //     }
    // }
}

// int StaticBuffer::getStaticBlockType(int blockNum){
//     // Check if blockNum is valid (non zero and less than number of disk blocks)
//     // and return E_OUTOFBOUND if not valid.
//     if(blockNum<0||blockNum>=DISK_BLOCKS){
//         return E_OUTOFBOUND;
//     }


//     // Access the entry in block allocation map corresponding to the blockNum argument
//     // and return the block type after type casting to integer.

//     int bType=(int)blockAllocMap[blockNum];
//     return bType;
// }

int StaticBuffer::getBufferNum(int blockNum){
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.
    if(blockNum<0||blockNum>=DISK_BLOCKS){
        return E_OUTOFBOUND;
    }

    // traverse through the metaInfo array and
    //  find the buffer number of the buffer to which the block is loaded.
    // if found return buffer number
    for(int buffIndex=0;buffIndex<BUFFER_CAPACITY;buffIndex++){
        if(metainfo[buffIndex].blockNum==blockNum){
            return buffIndex;
        }
    }
    // if block not found in buffer return E_BLOCKNOTINBUFFER
    return E_BLOCKNOTINBUFFER;

}


int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().
    int bufferIndex=StaticBuffer::getBufferNum(blockNum);


    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER
    if(bufferIndex==E_BLOCKNOTINBUFFER){
        return E_BLOCKNOTINBUFFER;
    }

    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND
    if(bufferIndex==E_OUTOFBOUND){
        return E_OUTOFBOUND;
    }

    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo
    metainfo[bufferIndex].dirty=true;
    

    // return SUCCESS
    return SUCCESS;
}

int StaticBuffer::getFreeBuffer(int blockNum){
    // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.
    if(blockNum<0||blockNum>=DISK_BLOCKS){
        return E_OUTOFBOUND;
    }

    // increase the timeStamp in metaInfo of all occupied buffers.
    for(int buffIn=0;buffIn<BUFFER_CAPACITY;buffIn++){
        if(metainfo[buffIn].free==false){
            metainfo[buffIn].timeStamp++;
        }
    }

    // let bufferNum be used to store the buffer number of the free/freed buffer.
    int bufferNum;

    // iterate through metainfo and check if there is any buffer free
    int buffIn;
    for(buffIn=0;buffIn<BUFFER_CAPACITY;buffIn++){
        if(metainfo[buffIn].free==true){
            bufferNum=buffIn;
            break;
        }
    }

    // if a free buffer is available, set bufferNum = index of that free buffer.
    

    // if a free buffer is not available,
    //     find the buffer with the largest timestamp
    //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    //     set bufferNum = index of this buffer
    if(buffIn==BUFFER_CAPACITY){
        int max=-1;
        for(int buffIndex=0;buffIndex<BUFFER_CAPACITY;buffIndex++){
            if(metainfo[buffIndex].timeStamp>max){
                max=metainfo[buffIndex].timeStamp;
                bufferNum=buffIndex;
            }
        }
        if(metainfo[bufferNum].dirty){
            Disk::writeBlock(StaticBuffer::blocks[bufferNum],metainfo[bufferNum].blockNum);
        }
    }

    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.
    metainfo[bufferNum].blockNum=blockNum;
    metainfo[bufferNum].free=false;
    metainfo[bufferNum].dirty=false;
    metainfo[bufferNum].timeStamp=0;

    // return the bufferNum.
    return bufferNum;
}



