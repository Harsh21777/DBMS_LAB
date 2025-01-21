#include "RelCacheTable.h"

#include <cstring>


RelCacheEntry *RelCacheTable::relCache[MAX_OPEN];

int RelCacheTable::getRelCatEntry(int relId, RelCatEntry *relCatBuf) {

  if(relId<0||relId>=MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if(relCache[relId]==nullptr) {
    return E_RELNOTOPEN;
  }

  // copy the corresponding Relation Catalog entry in the Relation Cache Table
  // to relCatBuf.
    *relCatBuf = relCache[relId]->relCatEntry;

  return SUCCESS;
}

// int RelCacheTable::setRelCatEntry(int relId, RelCatEntry *relCatBuf) {

//   if(relId<0||relId>=MAX_OPEN) {
//     return E_OUTOFBOUND;
//   }

//   if(relCache[relId]==nullptr) {
//     return E_RELNOTOPEN;
//   }
                                                                                                //NOT FOR STAGE-3
//   // copy the relCatBuf to the corresponding Relation Catalog entry in
//   // the Relation Cache Table.
//   relCache[relId]->relCatEntry=*relCatBuf;

//   // set the dirty flag of the corresponding Relation Cache entry in
//   // the Relation Cache Table.
//   relCache[relId]->dirty=true;

//   return SUCCESS;
// }

void RelCacheTable::recordToRelCatEntry(union Attribute record[RELCAT_NO_ATTRS],RelCatEntry* relCatEntry) {
    strcpy(relCatEntry->relName, record[RELCAT_REL_NAME_INDEX].sVal);
    relCatEntry->numAttrs = (int)record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    relCatEntry->numRecs=(int)record[RELCAT_NO_RECORDS_INDEX].nVal;
    relCatEntry->firstBlk=(int)record[RELCAT_FIRST_BLOCK_INDEX].nVal;
    relCatEntry->lastBlk=(int)record[RELCAT_LAST_BLOCK_INDEX].nVal;
    relCatEntry->numSlotsPerBlk=(int)record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal;
  /* fill the rest of the relCatEntry struct with the values at
      RELCAT_NO_RECORDS_INDEX,
      RELCAT_FIRST_BLOCK_INDEX,
      RELCAT_LAST_BLOCK_INDEX,
      RELCAT_NO_SLOTS_PER_BLOCK_INDEX
  */
}
