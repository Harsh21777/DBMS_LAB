#include "OpenRelTable.h"
#include <stdlib.h>
#include <cstring>

// OpenRelTable::OpenRelTable() {
//   // initialize relCache and attrCache with nullptr
//   for (int i = 0; i < MAX_OPEN; ++i) {
//     RelCacheTable::relCache[i] = nullptr;
//     AttrCacheTable::attrCache[i] = nullptr;
//   }
//   /************ Setting up Relation Cache entries ************/
//   // (we need to populate relation cache with entries for the relation catalog
//   //  and attribute catalog.)
//   /**** setting up Relation Catalog relation in the Relation Cache Table****/
//   RecBuffer relCatBlock(RELCAT_BLOCK);
//   Attribute relCatRecord[RELCAT_NO_ATTRS];
//   relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);
//   struct RelCacheEntry relCacheEntry;
//   RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
//   relCacheEntry.recId.block = RELCAT_BLOCK;
//   relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;
//   // allocate this on the heap because we want it to persist outside this function
//   RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
//   *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

//   /**** setting up Attribute Catalog relation in the Relation Cache Table ****/
//   relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);
//   struct RelCacheEntry attrCatRelEntry;
//   RelCacheTable::recordToRelCatEntry(relCatRecord, &attrCatRelEntry.relCatEntry);
//   attrCatRelEntry.recId.block = RELCAT_BLOCK;
//   attrCatRelEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
//   RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
//   *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatRelEntry;

//   /************ Setting up Attribute cache entries ************/
//   /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
//   RecBuffer attrCatBlock(ATTRCAT_BLOCK);
//   Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

//   // Create linked list for relation catalog attributes (slots 0-5)
//   struct AttrCacheEntry* head = nullptr;
//   struct AttrCacheEntry* current = nullptr;

//   for(int i = 0; i < 6; i++) {
//     attrCatBlock.getRecord(attrCatRecord, i);
//     struct AttrCacheEntry* newEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

//     AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &newEntry->attrCatEntry);
//     newEntry->recId.block = ATTRCAT_BLOCK;
//     newEntry->recId.slot = i;
//     newEntry->next = nullptr;

//     if(head == nullptr) {
//       head = newEntry;
//       current = newEntry;
//     } else {
//       current->next = newEntry;
//       current = newEntry;
//     }
//   }
//   AttrCacheTable::attrCache[RELCAT_RELID] = head;

//   /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/
//   // Create linked list for attribute catalog attributes (slots 6-11)
//   head = nullptr;
//   current = nullptr;

//   for(int i = 6; i < 12; i++) {
//     attrCatBlock.getRecord(attrCatRecord, i);
//     struct AttrCacheEntry* newEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

//     AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &newEntry->attrCatEntry);
//     newEntry->recId.block = ATTRCAT_BLOCK;
//     newEntry->recId.slot = i;
//     newEntry->next = nullptr;

//     if(head == nullptr) {
//       head = newEntry;
//       current = newEntry;
//     } else {
//       current->next = newEntry;
//       current = newEntry;
//     }
//   }
//   AttrCacheTable::attrCache[ATTRCAT_RELID] = head;
// }

AttrCacheEntry *createAttrCacheEntryList(int size)
{
    AttrCacheEntry *head = nullptr, *curr = nullptr;
    head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    size--;
    while (size--)
    {
        curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        curr = curr->next;
    }
    curr->next = nullptr;

    return head;
}

OpenRelTable::OpenRelTable()
{

    // initialize relCache and attrCache with nullptr
    for (int i = 0; i < MAX_OPEN; ++i)
    {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }

    /************ Setting up Relation Cache entries ************/
    // (we need to populate relation cache with entries for the relation catalog
    //  and attribute catalog.)

    // setting up the variables
    RecBuffer relCatBlock(RELCAT_BLOCK);
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    RelCacheEntry *relCacheEntry = nullptr;

    for (int relId = RELCAT_RELID; relId <= ATTRCAT_RELID + 2; relId++)
    {
        relCatBlock.getRecord(relCatRecord, relId);

        relCacheEntry = (RelCacheEntry *)malloc(sizeof(RelCacheEntry));
        RelCacheTable::recordToRelCatEntry(relCatRecord, &(relCacheEntry->relCatEntry));
        relCacheEntry->recId.block = RELCAT_BLOCK;
        relCacheEntry->recId.slot = relId;

        RelCacheTable::relCache[relId] = relCacheEntry;
    }

    /************ Setting up Attribute cache entries ************/
    // (we need to populate attribute cache with entries for the relation catalog
    //  and attribute catalog.)

    // setting up the variables
    RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;

    for (int relId = RELCAT_RELID, recordId = 0; relId <= ATTRCAT_RELID + 2; relId++)
    {
        int numberOfAttributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
        head = createAttrCacheEntryList(numberOfAttributes);
        attrCacheEntry = head;

        while (numberOfAttributes--)
        {
            attrCatBlock.getRecord(attrCatRecord, recordId);

            AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&(attrCacheEntry->attrCatEntry));
            attrCacheEntry->recId.slot = recordId++;
            attrCacheEntry->recId.block = ATTRCAT_BLOCK;

            attrCacheEntry = attrCacheEntry->next;
        }

        AttrCacheTable::attrCache[relId] = head;
    }
}

OpenRelTable::~OpenRelTable()
{
    // Free memory for relation cache entries
    for (int i = 0; i < MAX_OPEN; i++)
    {
        if (RelCacheTable::relCache[i] != nullptr)
        {
            free(RelCacheTable::relCache[i]);
            RelCacheTable::relCache[i] = nullptr;
        }
    }

    // Free memory for attribute cache entries
    for (int i = 0; i < MAX_OPEN; i++)
    {
        struct AttrCacheEntry *current = AttrCacheTable::attrCache[i];
        while (current != nullptr)
        {
            struct AttrCacheEntry *next = current->next;
            free(current);
            current = next;
        }
        AttrCacheTable::attrCache[i] = nullptr;
    }
}