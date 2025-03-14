#include "OpenRelTable.h"
#include <cstring>
#include <iostream>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];


AttrCacheEntry *createAttrCacheEntryList(int size) {
    AttrCacheEntry *head = nullptr, *curr = nullptr;
    head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    size--;
    while (size--) {
      curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
      curr = curr->next;
    }
    curr->next = nullptr;
    return head;
  }
  
OpenRelTable::OpenRelTable()
  {
    // initialise all values in relCache and attrCache to be 
    // nullptr and all entries in tableMetaInfo to be free
    for (int i = 0; i < MAX_OPEN; ++i)
    {
      RelCacheTable::relCache[i] = nullptr;
      AttrCacheTable::attrCache[i] = nullptr;
      tableMetaInfo[i].free = true;
    }
  
    // load the relation and attribute catalog into the relation cache (we did this already)
    
    // setting up the variables
    RecBuffer relCatBlock(RELCAT_BLOCK);
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    RelCacheEntry *relCacheEntry = nullptr;
  
    for (int relId = RELCAT_RELID; relId <= ATTRCAT_RELID; relId++)
    {
      relCatBlock.getRecord(relCatRecord, relId);
  
      relCacheEntry = (RelCacheEntry *)malloc(sizeof(RelCacheEntry));
      RelCacheTable::recordToRelCatEntry(relCatRecord, &(relCacheEntry->relCatEntry));
      relCacheEntry->recId.block = RELCAT_BLOCK;
      relCacheEntry->recId.slot = relId;
  
      relCacheEntry->searchIndex = {-1, -1};
  
      RelCacheTable::relCache[relId] = relCacheEntry;
    }
  
    // load the relation and attribute catalog into the attribute cache (we did this already)
    
    // setting up the variables
    RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;
  
    for (int relId = RELCAT_RELID, recordId = 0; relId <= ATTRCAT_RELID; relId++)
    {
      int numberOfAttributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
      head = createAttrCacheEntryList(numberOfAttributes);
      attrCacheEntry = head;
  
      while (numberOfAttributes--)
      {
        attrCatBlock.getRecord(attrCatRecord, recordId);
  
        AttrCacheTable::recordToAttrCatEntry(
          attrCatRecord,
          &(attrCacheEntry->attrCatEntry));
        attrCacheEntry->recId.slot = recordId++;
        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
  
        attrCacheEntry = attrCacheEntry->next;
      }
  
      AttrCacheTable::attrCache[relId] = head;
    }
  
    /************ Setting up tableMetaInfo entries ************/
  
    tableMetaInfo[RELCAT_RELID].free = false; 
    strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);
  
    tableMetaInfo[ATTRCAT_RELID].free = false; 
    strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}
  

OpenRelTable::~OpenRelTable()
{
	// free all the memory that you allocated in the constructor

	//? close all open relations (from rel-id = 2 onwards. Why?)
	for (int i = 2; i < MAX_OPEN; ++i)
		if (!tableMetaInfo[i].free)
			OpenRelTable::closeRel(i); // we will implement this function later

	if(RelCacheTable::relCache[ATTRCAT_RELID]->dirty==true){
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(ATTRCAT_RELID,&relCatBuffer);
		Attribute relCatRecord[RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer,relCatRecord);
		RecId recId=RelCacheTable::relCache[ATTRCAT_RELID]->recId;
		RecBuffer relCatBlock(recId.block);
		relCatBlock.setRecord(relCatRecord,recId.slot);
	}
	free(RelCacheTable::relCache[ATTRCAT_RELID]);

	if(RelCacheTable::relCache[RELCAT_RELID]->dirty==true){
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(RELCAT_RELID,&relCatBuffer);
		Attribute relCatRecord[ATTRCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer,relCatRecord);
		RecId recId=RelCacheTable::relCache[RELCAT_RELID]->recId;
		RecBuffer attrCatRecord(recId.block);
		attrCatRecord.setRecord(relCatRecord,recId.slot);

	}
	free(RelCacheTable::relCache[RELCAT_RELID]);
	// free the memory allocated for rel-id 0 and 1 in the caches
	for(int relID=RELCAT_RELID;relID<=ATTRCAT_RELID;relID++){
		AttrCacheEntry *curr=AttrCacheTable::attrCache[relID],*next=NULL;
		while(curr!=nullptr){
			next=curr->next;
			if(curr->dirty==true){
				AttrCatEntry attrCatEntry=curr->attrCatEntry;
				Attribute AttrCatrecord[ATTRCAT_NO_ATTRS];
				AttrCacheTable::attrCatEntryToRecord(&attrCatEntry,AttrCatrecord);
				RecBuffer attrCatBlock(curr->recId.block);
				attrCatBlock.setRecord(AttrCatrecord,curr->recId.slot);
			}
			free(curr);
			curr=next;
		}
	}
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE])
{
    for(int i=0;i<MAX_OPEN;i++){
      if(strcmp(relName,tableMetaInfo[i].relName)==0&&tableMetaInfo[i].free==false){
        return i;
      }
    }    

    return E_RELNOTOPEN;
}

int OpenRelTable::getFreeOpenRelTableEntry() {

    /* traverse through the tableMetaInfo array,
      find a free entry in the Open Relation Table.*/
    for(int i=0;i<MAX_OPEN;i++){
        if(tableMetaInfo[i].free==true){
            return i;
        }
    }

    return E_CACHEFULL;
  
    // if found return the relation id, else return E_CACHEFULL.
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {

    int ret=OpenRelTable::getRelId(relName);
    if(ret>=0){
      // (checked using OpenRelTable::getRelId())
        return ret;
      // return that relation id;
    }
  
    /* find a free slot in the Open Relation Table
       using OpenRelTable::getFreeOpenRelTableEntry(). */
    int free_sl=OpenRelTable::getFreeOpenRelTableEntry();
  
    if (free_sl==E_CACHEFULL){
      return E_CACHEFULL;
    }
  
    // let relId be used to store the free slot.
    int relId;

    relId=free_sl;
  
    /****** Setting up Relation Cache entry for the relation ******/
  
    /* search for the entry with relation name, relName, in the Relation Catalog using
        BlockAccess::linearSearch().
        Care should be taken to reset the searchIndex of the relation RELCAT_RELID
        before calling linearSearch().*/
    
    
    Attribute attrVal;
    strcpy(attrVal.sVal, relName);
 
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
  
    // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
    RecId relcatRecId;
    char reln[]="RelName";
    relcatRecId=BlockAccess::linearSearch(RELCAT_RELID,reln,attrVal,EQ);
    if ( relcatRecId.block==-1&&relcatRecId.slot==-1) {
      // (the relation is not found in the Relation Catalog.)
      return E_RELNOTEXIST;
    }
  
    /* read the record entry corresponding to relcatRecId and create a relCacheEntry
        on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
        update the recId field of this Relation Cache entry to relcatRecId.
        use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
      NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
    */
   RecBuffer recBuff(relcatRecId.block);
   Attribute relRec[RELCAT_NO_ATTRS];

   RelCacheEntry* relCacheBuff =nullptr;
   relCacheBuff=(RelCacheEntry*)malloc(sizeof(RelCacheEntry));

   recBuff.getRecord(relRec,relcatRecId.slot);

   RelCacheTable::recordToRelCatEntry(relRec,&(relCacheBuff->relCatEntry));

   relCacheBuff->recId=relcatRecId;
   RelCacheTable::relCache[relId]=relCacheBuff;
  
    /****** Setting up Attribute Cache entry for the relation ******/
  
    // let listHead be used to hold the head of the linked list of attrCache entries.
    AttrCacheEntry* listHead=nullptr;
    AttrCacheEntry* attrCatBuff=nullptr;
    attrCatBuff=(AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    // attrCatBuff=nullptr;

    int numAttr=RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
    listHead=createAttrCacheEntryList(numAttr);
    attrCatBuff=listHead;
  
    /*iterate over all the entries in the Attribute Catalog corresponding to each
    attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
    care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
    corresponding to Attribute Catalog before the first call to linearSearch().*/
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    for(int i=0;i<numAttr;i++)
    {
        /* let attrcatRecId store a valid record id an entry of the relation, relName,
        in the Attribute Catalog.*/
        RecId attrcatRecId;
        attrcatRecId=BlockAccess::linearSearch(ATTRCAT_RELID,reln,attrVal,EQ);
  
        /* read the record entry corresponding to attrcatRecId and create an
        Attribute Cache entry on it using RecBuffer::getRecord() and
        AttrCacheTable::recordToAttrCatEntry().
        update the recId field of this Attribute Cache entry to attrcatRecId.
        add the Attribute Cache entry to the linked list of listHead .*/
        // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()

        RecBuffer attrBuff(attrcatRecId.block);
        Attribute atb[ATTRCAT_NO_ATTRS];
        attrBuff.getRecord(atb,attrcatRecId.slot);
        
        AttrCacheTable::recordToAttrCatEntry(atb,&(attrCatBuff->attrCatEntry));
        attrCatBuff->recId=attrcatRecId;
        attrCatBuff=attrCatBuff->next;
    }
  
    // set the relIdth entry of the AttrCacheTable to listHead.
    AttrCacheTable::attrCache[relId]=listHead;
    /****** Setting up metadata in the Open Relation Table for the relation******/
  
    // update the relIdth entry of the tableMetaInfo with free as false and
    // relName as the input.
    tableMetaInfo[relId].free=false;
    strcpy(tableMetaInfo[relId].relName,relName);
  
    return relId;
}


int OpenRelTable::closeRel(int relId) {
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) return E_NOTPERMITTED;

  if (0 > relId || relId >= MAX_OPEN) return E_OUTOFBOUND;

  if (tableMetaInfo[relId].free) return E_RELNOTOPEN;

if (RelCacheTable::relCache[relId]->dirty == true) {
  /* Get the Relation Catalog entry from RelCacheTable::relCache
  Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
  Attribute relCatBuffer[RELCAT_NO_ATTRS];
  RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry), relCatBuffer);

  // declaring an object of RecBuffer class to write back to the buffer
  RecId rec = RelCacheTable::relCache[relId]->recId;
  RecBuffer relCatBlock(rec.block);

  // Write back to the buffer using relCatBlock.setRecord() with recId.slot
  relCatBlock.setRecord(relCatBuffer, RelCacheTable::relCache[relId]->recId.slot);
}

// free the memory allocated in the relation and attribute caches which was
// allocated in the OpenRelTable::openRel() function
free (RelCacheTable::relCache[relId]);

// // RelCacheEntry *relCacheBuffer = RelCacheTable::relCache[relId];

//* because we are not modifying the attribute cache at this stage,
//* write-back is not required. We will do it in subsequent
  //* stages when it becomes needed)

AttrCacheEntry *head = AttrCacheTable::attrCache[relId];
AttrCacheEntry *next = head->next;

while (next) {
  free (head);
  head = next;
  next = next->next;
}
free(head);	

// update `tableMetaInfo` to set `relId` as a free slot
// update `relCache` and `attrCache` to set the entry at `relId` to nullptr
tableMetaInfo[relId].free = true;
RelCacheTable::relCache[relId] = nullptr;
AttrCacheTable::attrCache[relId] = nullptr;

return SUCCESS;
}