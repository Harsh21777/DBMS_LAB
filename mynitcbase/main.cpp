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
  // Disk disk_run;
  // /* Initialize the Run Copy of Disk */
  // StaticBuffer buffer;
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

  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  /*
  for i = 0 and i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)

      get the relation catalog entry using RelCacheTable::getRelCatEntry()
      printf("Relation: %s\n", relname);

      for j = 0 to numAttrs of the relation - 1
          get the attribute catalog entry for (rel-id i, attribute offset j)
           in attrCatEntry using AttrCacheTable::getAttrCatEntry()

          printf("  %s: %s\n", attrName, attrType);
  */

  for (int i = 0; i <= 2; i++)
  {
    RelCatEntry *relCatBuf = (RelCatEntry *)malloc(sizeof(RelCatEntry));
    RelCacheTable::getRelCatEntry(i, relCatBuf);

    printf("Relation: %s\n", relCatBuf->relName);

    for (int j = 0; j < relCatBuf->numAttrs; j++)
    {

      AttrCatEntry *attrCatBuf = (AttrCatEntry *)malloc(sizeof(AttrCatEntry));

      AttrCacheTable::getAttrCatEntry(i, j, attrCatBuf);

      printf(" %s: %s\n", attrCatBuf->attrName, (attrCatBuf->attrType == NUMBER) ? "NUM" : "STR");
    }

    printf("\n");
  }

  

  return 0;

  // StaticBuffer buffer;
  // OpenRelTable cache;

  // return FrontendInterface::handleFrontend(argc, argv);
}
