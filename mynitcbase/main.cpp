#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <cstring>
using namespace std;
#define BLOCK_SIZE 2048

int main(int argc, char *argv[])
{

  /* Initialize the Run Copy of Disk */
  Disk disk_run;

  // unsigned char buffer[BLOCK_SIZE];
  // Disk::readBlock(buffer,7000);
  // char msg[]="hello";
  // memcpy(buffer+20,msg,6);
  // Disk::writeBlock(buffer,7000;
  // STAGE-1 EX
  // unsigned char buffer2[BLOCK_SIZE];
  // char msg2[6];
  // Disk::readBlock(buffer,7000);
  // memcpy(msg2,buffer+20,6);
  // Disk::writeBlock(buffer,7000);

  // cout<<msg2;

  // unsigned char buffer[BLOCK_SIZE];
  // for(int i=0;i<=3;i++){
  //   Disk::readBlock(buffer,i);               //STAGE-1 EXE
  //   for(auto x:buffer){
  //     printf("%d",x);
  //   }
  //   cout<<endl;
  // }

  // unsigned char buffer[BLOCK_SIZE];
  // Disk::readBlock(buffer,0);
  // for(int i=0;i<100;i++){                        //STAGE-1 EXE
  // cout<<(int)buffer[i];
  // }

  // STAGE-2

  // EXAMPLE Q

  // RecBuffer relCatBuffer(RELCAT_BLOCK);
  // RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  // HeadInfo relCatHeader;
  // HeadInfo attrCatHeader;
  // // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // // (we will implement these functions later)
  // relCatBuffer.getHeader(&relCatHeader);
  // attrCatBuffer.getHeader(&attrCatHeader);

  // for (int i=0;i<relCatHeader.numEntries;i++)
  // {

  //   Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog
  //   relCatBuffer.getRecord(relCatRecord, i);

  //   printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

  //   for (int j=0;j<attrCatHeader.numEntries;j++)
  //   {

  //     // declare attrCatRecord and load the attribute catalog entry into it
  //     Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  //     attrCatBuffer.getRecord(attrCatRecord,j);
  //     if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0)
  //     {

  //       const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
  //       printf("  %s: %s\n",attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
  //     }
  //   }

  //   printf("\n");

  // }

  // Q-1
  // int currentBlock;
  // RecBuffer relCatBuffer(RELCAT_BLOCK);
  // RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  // HeadInfo relCatheader;
  // HeadInfo attrCatHeader;

  // relCatBuffer.getHeader(&relCatheader);
  

  // for (int i = 0; i < relCatheader.numEntries; i++)
  // {
  //   Attribute relCatRecord[RELCAT_NO_ATTRS]; // printing the relations
  //   relCatBuffer.getRecord(relCatRecord, i);
  //   printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

  //   currentBlock = ATTRCAT_BLOCK;
  //   while (currentBlock != -1)
  //   {

  //     RecBuffer attrCatBuffer(currentBlock);
  //     attrCatBuffer.getHeader(&attrCatHeader);

  //     for (int j = 0; j < attrCatHeader.numEntries; j++)
  //     {
  //       Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  //       attrCatBuffer.getRecord(attrCatRecord, j);
  //       if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0)
  //       {
  //         const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
  //         printf(" %s:%s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
  //       }
  //     }
  //     currentBlock = attrCatHeader.rblock;
  //   }
  // }

  // Q-2

  int currentblock;
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  relCatBuffer.getHeader(&relCatHeader);
  

  for (int i = 0; i < relCatHeader.numEntries; i++)
  {
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBuffer.getRecord(relCatRecord, i);
    if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, "Students") == 0)
    {
      printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
      currentblock = ATTRCAT_BLOCK;
      while (currentblock != -1)
      {
        RecBuffer attrCatBuffer(currentblock);
        attrCatBuffer.getHeader(&attrCatHeader);
        for (int j = 0; j < attrCatHeader.numEntries; j++)
        {
          Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
          attrCatBuffer.getRecord(attrCatRecord, j);
          if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0)
          {
            if (strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Class") == 0)
              strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Batch");
            attrCatBuffer.setRecord(attrCatRecord, j);
          }
          if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0)
          {
            const char *attrtype = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
            printf(" %s:%s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrtype);
          }
        }
        currentblock = attrCatHeader.rblock;
      }
    }
  }

  return 0;

  // StaticBuffer buffer;
  // OpenRelTable cache;

  // return FrontendInterface::handleFrontend(argc, argv);
}
