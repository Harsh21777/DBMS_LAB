#include "BlockAccess.h"
#include <iostream>
#include <cstring>
#include <stdlib.h>

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE],union Attribute attrVal, int op) {

    // get the previous search index of the relation relId from the relation cache
    // (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);
    // let block and slot denote the record id of the record being currently
    // checked
    int block = -1, slot = -1;
    // if the current search index record is invalid(i.e. both block and slot =
    // -1)
    if (prevRecId.block == -1 && prevRecId.slot == -1) {
      // (no hits from previous search; search should start from the
      // first record itself)
      // get the first record block of the relation from the relation cache
      // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
      RelCatEntry RelCatBuf;
      RelCacheTable::getRelCatEntry(relId, &RelCatBuf);
      // block = first record block of the relation
      block = RelCatBuf.firstBlk;
      slot = 0;
      // slot = 0
    } else {
      // (there is a hit from previous search; search should start from
      // the record next to the search index record)
  
      // block = search index's block
      // slot = search index's slot + 1
      block = prevRecId.block;
      slot = prevRecId.slot + 1;
    }
  
    /* The following code searches for the next record in the relation
       that satisfies the given condition
       We start from the record id (block, slot) and iterate over the remaining
       records of the relation
    */
    RelCatEntry relCatBuffer;
    RelCacheTable::getRelCatEntry(relId, &relCatBuffer);
    while (block != -1) {
      /* create a RecBuffer object for block (use RecBuffer Constructor for
         existing block) */
      RecBuffer Buffer(block);
  
      HeadInfo header;
      Attribute CatRecord[RELCAT_NO_ATTRS];
  
      // get the record with id (block, slot) using RecBuffer::getRecord()
      Buffer.getRecord(CatRecord, slot);
      // get header of the block using RecBuffer::getHeader() function
      Buffer.getHeader(&header);
      // get slot map of the block using RecBuffer::getSlotMap() function
      unsigned char slotMap[header.numSlots];
      Buffer.getSlotMap(slotMap);
      // If slot >= the number of slots per block(i.e. no more slots in this
      // block)
      // if (slot >= header.numSlots) {
      //   // update block = right block of block
      //   block = header.rblock;
      //   slot = 0;
      //   // update slot = 0
      //   continue; // continue to the beginning of this while loop
      // }
  
      if (slot >= relCatBuffer.numSlotsPerBlk) {
        block = header.rblock, slot = 0;
        continue; // continue to the beginning of this while loop
      }
  
      // if slot is free skip the loop
      // (i.e. check if slot'th entry in slot map of block contains
      // SLOT_UNOCCUPIED)
      if (slotMap[slot] == SLOT_UNOCCUPIED) {
        slot++;
        continue;
        // increment slot and continue to the next record slot
      }
  
      // compare record's attribute value to the the given attrVal as below:
      /*
          firstly get the attribute offset for the attrName attribute
          from the attribute cache entry of the relation using
          AttrCacheTable::getAttrCatEntry()
      */
      AttrCatEntry attrCatBuf;
      AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuf);
  
      /* use the attribute offset to get the value of the attribute from
         current record */
      Attribute record[header.numAttrs];
      Buffer.getRecord(record, slot);
      int attrOffset = attrCatBuf.offset;
  
      int cmpVal = compareAttrs(record[attrOffset], attrVal,attrCatBuf.attrType); // will store the difference between the attributes
      // set cmpVal using compareAttrs()
      /* Next task is to check whether this record satisfies the given condition.
         It is determined based on the output of previous comparison and
         the op value received.
         The following code sets the cond variable if the condition is satisfied.
      */
      if ((op == NE && cmpVal != 0) || // if op is "not equal to"
          (op == LT && cmpVal < 0) ||  // if op is "less than"
          (op == LE && cmpVal <= 0) || // if op is "less than or equal to"
          (op == EQ && cmpVal == 0) || // if op is "equal to"
          (op == GT && cmpVal > 0) ||  // if op is "greater than"
          (op == GE && cmpVal >= 0)    // if op is "greater than or equal to"
      ) {
        /*
        set the search index in the relation cache as
        the record id of the record that satisfies the given condition
        (use RelCacheTable::setSearchIndex function)
        */
        RecId newIndex;
        newIndex.block = block;
        newIndex.slot = slot;
        RelCacheTable::setSearchIndex(relId, &newIndex);
        return RecId{block, slot};
      }
      slot++;
    }
  
    // no record in the relation with Id relid satisfies the given condition
    return RecId{-1, -1};
  }
  

int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
       RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute newRelationName;    // set newRelationName with newName
    strcpy(newRelationName.sVal,newName);
    // search the relation catalog for an entry with "RelName" = newRelationName
    char reln[]="RelName";

    RecId newRec=BlockAccess::linearSearch(RELCAT_RELID,reln,newRelationName,EQ);


    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;
    if(newRec.block!=-1&&newRec.slot!=-1){
        return E_RELEXIST;
    }


    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute oldRelationName;    // set oldRelationName with oldName
    strcpy(oldRelationName.sVal,oldName);

    // search the relation catalog for an entry with "RelName" = oldRelationName
    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;
    RecId oldRec=BlockAccess::linearSearch(RELCAT_RELID,reln,oldRelationName,EQ);
    if(oldRec.block==-1&&oldRec.slot==-1){
        return E_RELNOTEXIST;
    }

    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord
    RecBuffer oldBuff(oldRec.block);
    Attribute oldatb[RELCAT_NO_ATTRS];
    oldBuff.getRecord(oldatb,oldRec.slot);

    strcpy(oldatb[RELCAT_REL_NAME_INDEX].sVal,newName);

    oldBuff.setRecord(oldatb,oldRec.slot);


    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    int numat=oldatb[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    

    //for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord

    for(int i=0;i<=numat;i++){
        RecId tempRec=BlockAccess::linearSearch(ATTRCAT_RELID,reln,oldRelationName,EQ);
        if(tempRec.block==-1||tempRec.slot==-1){
            continue;
        }
        RecBuffer tempBuff(tempRec.block);
        Attribute tempatb[ATTRCAT_NO_ATTRS];

        tempBuff.getRecord(tempatb,tempRec.slot);
        strcpy(tempatb[ATTRCAT_REL_NAME_INDEX].sVal,newName);
        tempBuff.setRecord(tempatb,tempRec.slot);
    }

    return SUCCESS;
}


int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr;    // set relNameAttr to relName
    strcpy(relNameAttr.sVal,relName);

    // Search for the relation with name relName in relation catalog using linearSearch()
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
    char RELN[]="RelName";
    RecId relRec=BlockAccess::linearSearch(RELCAT_RELID,RELN,relNameAttr,EQ);
    if(relRec.block==-1&&relRec.slot==-1){
        
        return E_RELNOTEXIST;
    }

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId{-1, -1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
    while (true) {
        // linear search on the attribute catalog for RelName = relNameAttr
        RecId tempRec=BlockAccess::linearSearch(ATTRCAT_RELID,RELN,relNameAttr,EQ);

        // if there are no more attributes left to check (linearSearch returned {-1,-1})
        //     break;
        if(tempRec.block==-1&&tempRec.slot==-1){
            break;
        }

        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */
        RecBuffer attrBuf(tempRec.block);
        attrBuf.getRecord(attrCatEntryRecord,tempRec.slot);

        // if attrCatEntryRecord.attrName = oldName
        //     attrToRenameRecId = block and slot of this record
        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,oldName)==0){
            attrToRenameRecId.block=tempRec.block;
            attrToRenameRecId.slot=tempRec.slot;
            break;
        }

        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;
        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName)==0){
            return E_ATTREXIST;
        }
    }

    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;
    if(attrToRenameRecId.block==-1&&attrToRenameRecId.slot==-1){
        return E_ATTRNOTEXIST;
    }


    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord
    RecBuffer newBuf(attrToRenameRecId.block);
    Attribute newatb[ATTRCAT_NO_ATTRS];
    newBuf.getRecord(newatb,attrToRenameRecId.slot);
    strcpy(newatb[ATTRCAT_ATTR_NAME_INDEX].sVal,newName);
    newBuf.setRecord(newatb,attrToRenameRecId.slot);

    return SUCCESS;
}

int BlockAccess::insert(int relId, Attribute *record) {
    // get the relation catalog entry from relation cache
    // ( use RelCacheTable::getRelCatEntry() of Cache Layer)
    
    RelCatEntry relEntry;
    RelCacheTable::getRelCatEntry(relId,&relEntry);

    int blockNum = relEntry.firstBlk;

    // rec_id will be used to store where the new record will be inserted
    RecId rec_id = {-1, -1};

    int numOfSlots = relEntry.numSlotsPerBlk;
    int numOfAttributes = relEntry.numAttrs;

    int prevBlockNum = -1;

    /*
        Traversing the linked list of existing record blocks of the relation
        until a free slot is found OR
        until the end of the list is reached
    */
    
    while (blockNum != -1) {
        // create a RecBuffer object for blockNum (using appropriate constructor!)
        // get header of block(blockNum) using RecBuffer::getHeader() function
        // get slot map of block(blockNum) using RecBuffer::getSlotMap() function
        
        RecBuffer rBuff(blockNum);
        HeadInfo head;
        rBuff.getHeader(&head);
        
        unsigned char slotmap[head.numSlots];
        rBuff.getSlotMap(slotmap);
        

        // search for free slot in the block 'blockNum' and store it's rec-id in rec_id
        // (Free slot can be found by iterating over the slot map of the block)
        /* slot map stores SLOT_UNOCCUPIED if slot is free and
           SLOT_OCCUPIED if slot is occupied) */
        int i=0;
        for(;i<numOfSlots;i++){
            if(slotmap[i]==SLOT_UNOCCUPIED){
               break;
            }
        }

        if(i!=numOfSlots){
            rec_id.block=blockNum;
            rec_id.slot=i;
            break;
        }

        prevBlockNum=blockNum;
        blockNum=head.rblock;
        

        /* if a free slot is found, set rec_id and discontinue the traversal
           of the linked list of record blocks (break from the loop) */

        /* otherwise, continue to check the next block by updating the
           block numbers as follows:
              update prevBlockNum = blockNum
              update blockNum = header.rblock (next element in the linked
                                               list of record blocks)
        */
    }



    //  if no free slot is found in existing record blocks (rec_id = {-1, -1})
    if(rec_id.block==-1&&rec_id.slot==-1)
    {
        // if relation is RELCAT, do not allocate any more blocks
        //     return E_MAXRELATIONS;
        if(relId==RELCAT_RELID){
            return E_MAXRELATIONS;
        }

        // Otherwise,
        // get a new record block (using the appropriate RecBuffer constructor!)
        // get the block number of the newly allocated block
        // (use BlockBuffer::getBlockNum() function)
        // let ret be the return value of getBlockNum() function call
        
        RecBuffer rBuff;
        
        int ret=rBuff.getBlockNum();
        if (ret == E_DISKFULL) {
            return E_DISKFULL;
        }

        // Assign rec_id.block = new block number(i.e. ret) and rec_id.slot = 0
        rec_id.block=ret;
        rec_id.slot=0;

        /*
            set the header of the new record block such that it links with
            existing record blocks of the relation
            set the block's header as follows:
            blockType: REC, pblock: -1
            lblock
                  = -1 (if linked list of existing record blocks was empty
                         i.e this is the first insertion into the relation)
                  = prevBlockNum (otherwise),
            rblock: -1, numEntries: 0,
            numSlots: numOfSlots, numAttrs: numOfAttributes
            (use BlockBuffer::setHeader() function)
        */
        HeadInfo head;
        head.blockType=REC;
        head.pblock=-1;
        head.rblock=-1;
        head.numEntries=0;
        head.numSlots=numOfSlots;
        head.numAttrs=numOfAttributes;
        if(relEntry.numRecs==0){
            head.lblock=-1;
        }
        else{
            head.lblock=prevBlockNum;
        }
        rBuff.setHeader(&head);

        /*
            set block's slot map with all slots marked as free
            (i.e. store SLOT_UNOCCUPIED for all the entries)
            (use RecBuffer::setSlotMap() function)
        */
        unsigned char slotMap[relEntry.numSlotsPerBlk];
        for(int i=0;i<numOfSlots;i++){
            slotMap[i]=SLOT_UNOCCUPIED;
        }
        rBuff.setSlotMap(slotMap);


        // if prevBlockNum != -1
        
        if(prevBlockNum!=-1)
        {
            // create a RecBuffer object for prevBlockNum
            // get the header of the block prevBlockNum and
            // update the rblock field of the header to the new block
            // number i.e. rec_id.block
            // (use BlockBuffer::setHeader() function)
            RecBuffer preBuff(prevBlockNum);
            HeadInfo head;
            preBuff.getHeader(&head);
            head.rblock=rec_id.block;
            preBuff.setHeader(&head);
        }
        else
        {
            // update first block field in the relation catalog entry to the
            // new block (using RelCacheTable::setRelCatEntry() function)
            relEntry.firstBlk=rec_id.block;
            RelCacheTable::setRelCatEntry(relId,&relEntry);
        }

        // update last block field in the relation catalog entry to the
        // new block (using RelCacheTable::setRelCatEntry() function)
        relEntry.lastBlk=rec_id.block;
        RelCacheTable::setRelCatEntry(relId,&relEntry);
    }

    // create a RecBuffer object for rec_id.block
    // insert the record into rec_id'th slot using RecBuffer.setRecord())
    RecBuffer buff(rec_id.block);
    buff.setRecord(record,rec_id.slot);

    /* update the slot map of the block by marking entry of the slot to
       which record was inserted as occupied) */
    // (ie store SLOT_OCCUPIED in free_slot'th entry of slot map)
    // (use RecBuffer::getSlotMap() and RecBuffer::setSlotMap() functions)
    
    unsigned char slotMAP[relEntry.numSlotsPerBlk];
    buff.getSlotMap(slotMAP);
    slotMAP[rec_id.slot]=SLOT_OCCUPIED;
    buff.setSlotMap(slotMAP);

    // increment the numEntries field in the header of the block to
    // which record was inserted
    // (use BlockBuffer::getHeader() and BlockBuffer::setHeader() functions)
    HeadInfo head;
    buff.getHeader(&head);
    head.numEntries+=1;
    buff.setHeader(&head);

    // Increment the number of records field in the relation cache entry for
    // the relation. (use RelCacheTable::setRelCatEntry function)
    relEntry.numRecs+=1;
    RelCacheTable::setRelCatEntry(relId,&relEntry);

   

    return SUCCESS;
}

/*
NOTE: This function will copy the result of the search to the `record` argument.
      The caller should ensure that space is allocated for `record` array
      based on the number of attributes in the relation.
*/
int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    // Declare a variable called recid to store the searched record
    /* search for the record id (recid) corresponding to the attribute with
    attribute name attrName, with value attrval and satisfying the condition op
    using linearSearch() */
    RecId recId=BlockAccess::linearSearch(relId,attrName,attrVal,op);

    if(recId.block==-1&&recId.slot==-1){
        return E_NOTFOUND;
    }

    // if there's no record satisfying the given condition (recId = {-1, -1})
    //    return E_NOTFOUND;

    /* Copy the record with record id (recId) to the record buffer (record)
       For this Instantiate a RecBuffer class object using recId and
       call the appropriate method to fetch the record
    */
    RecBuffer searchRec(recId.block);
    int check=searchRec.getRecord(record,recId.slot);
    if(check!=SUCCESS){
        return check;
    }

    return SUCCESS;
}

int BlockAccess::deleteRelation(char relName[ATTR_SIZE]) {
    // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    if(strcmp(RELCAT_RELNAME,relName)==0||strcmp(ATTRCAT_RELNAME,relName)==0){
        return E_NOTPERMITTED;
    }
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr; // (stores relName as type union Attribute)
    // assign relNameAttr.sVal = relName
    strcpy(relNameAttr.sVal,relName);
    char reln[]="RelName";

    RecId rec_id=BlockAccess::linearSearch(RELCAT_RELID,reln,relNameAttr,EQ);
    if(rec_id.block==-1&&rec_id.slot==-1){
        return E_RELNOTEXIST;
    }

    //  linearSearch on the relation catalog for RelName = relNameAttr

    // if the relation does not exist (linearSearch returned {-1, -1})
    //     return E_RELNOTEXIST

    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];
    /* store the relation catalog record corresponding to the relation in
       relCatEntryRecord using RecBuffer.getRecord */

    /* get the first record block of the relation (firstBlock) using the
       relation catalog entry record */
    /* get the number of attributes corresponding to the relation (numAttrs)
       using the relation catalog entry record */
    RecBuffer relRec(rec_id.block);
    int check=relRec.getRecord(relCatEntryRecord,rec_id.slot);
    if(check!=SUCCESS){
        return check;
    }
    int fstblk=relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;
    int numAttr=relCatEntryRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    int numSlots=relCatEntryRecord[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal;


    /*
     Delete all the record blocks of the relation
    */
    // for each record block of the relation:
    //     get block header using BlockBuffer.getHeader
    //     get the next block from the header (rblock)
    //     release the block using BlockBuffer.releaseBlock
    //
    //     Hint: to know if we reached the end, check if nextBlock = -1
    int blknum=fstblk;
    
    while(blknum!=-1){
        RecBuffer buff(blknum);
        HeadInfo head;
        check=buff.getHeader(&head);
        if(check!=SUCCESS){
            return check;
        }

        
        blknum=head.rblock;
        
        buff.releaseBlock();   
    }


    /***
        Deleting attribute catalog entries corresponding the relation and index
        blocks corresponding to the relation with relName on its attributes
    ***/

    // reset the searchIndex of the attribute catalog
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    int numberOfAttributesDeleted = 0;

    while(true) {
        RecId attrCatRecId;
        // attrCatRecId = linearSearch on attribute catalog for RelName = relNameAttr
        attrCatRecId=BlockAccess::linearSearch(ATTRCAT_RELID,reln,relNameAttr,EQ);

        // if no more attributes to iterate over (attrCatRecId == {-1, -1})
        //     break;
        if(attrCatRecId.block==-1&&attrCatRecId.slot==-1){
            break;
        }

        numberOfAttributesDeleted++;

        // create a RecBuffer for attrCatRecId.block
        // get the header of the block
        // get the record corresponding to attrCatRecId.slot

        // declare variable rootBlock which will be used to store the root
        // block field from the attribute catalog record.
        RecBuffer atbBuff(attrCatRecId.block);
        HeadInfo head;
        check=atbBuff.getHeader(&head);
        if(check!=SUCCESS){
            return check;
        }
        Attribute atb[ATTRCAT_NO_ATTRS];
        check=atbBuff.getRecord(atb,attrCatRecId.slot);
        if(check!=SUCCESS){
            return check;
        }

        int rootBlock = atb[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
        // (This will be used later to delete any indexes if it exists)

        // Update the Slotmap for the block by setting the slot as SLOT_UNOCCUPIED
        // Hint: use RecBuffer.getSlotMap and RecBuffer.setSlotMap
        unsigned char slotmap[head.numSlots];
        check=atbBuff.getSlotMap(slotmap);
        if(check!=SUCCESS){
            return check;
        }
        slotmap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
        check=atbBuff.setSlotMap(slotmap);
        if(check!=SUCCESS){
            return check;
        }

        head.numEntries-=1;
        check=atbBuff.setHeader(&head);
        if(check!=SUCCESS){
            return check;
        }
        /* Decrement the numEntries in the header of the block corresponding to
           the attribute catalog entry and then set back the header
           using RecBuffer.setHeader */

        /* If number of entries become 0, releaseBlock is called after fixing
           the linked list.
        */
        if (head.numEntries==0) {
            /* Standard Linked List Delete for a Block
               Get the header of the left block and set it's rblock to this
               block's rblock
            */

            // create a RecBuffer for lblock and call appropriate methods
            RecBuffer leftbuff(head.lblock);
            HeadInfo lefthead;
            check=leftbuff.getHeader(&lefthead);
            if(check!=SUCCESS){
                return check;
            }
            lefthead.rblock=head.rblock;
            check=leftbuff.setHeader(&lefthead);
            if(check!=SUCCESS){
                return check;
            }

            if (lefthead.rblock!=-1) {
                /* Get the header of the right block and set it's lblock to
                   this block's lblock */
                // create a RecBuffer for rblock and call appropriate methods
                RecBuffer rytbuff(head.rblock);
                HeadInfo rythead;
                check=rytbuff.getHeader(&rythead);
                if(check!=SUCCESS){
                    return check;
                }
                rythead.lblock=head.lblock;
                check=rytbuff.setHeader(&rythead);
                if(check!=SUCCESS){
                    return check;
                }


            } else {
                // (the block being released is the "Last Block" of the relation.)
                /* update the Relation Catalog entry's LastBlock field for this
                   relation with the block number of the previous block. */
                RelCatEntry relEntry;
                RelCacheTable::getRelCatEntry(ATTRCAT_RELID,&relEntry);
                relEntry.lastBlk=head.lblock;
            }

            // (Since the attribute catalog will never be empty(why?), we do not
            //  need to handle the case of the linked list becoming empty - i.e
            //  every block of the attribute catalog gets released.)
            atbBuff.releaseBlock();

            // call releaseBlock()
        }

        // (the following part is only relevant once indexing has been implemented)
        // if index exists for the attribute (rootBlock != -1), call bplus destroy
        // if (rootBlock != -1) {
        //     // delete the bplus tree rooted at rootBlock using BPlusTree::bPlusDestroy()
        // }
    }

    /*** Delete the entry corresponding to the relation from relation catalog ***/
    // Fetch the header of Relcat block
    HeadInfo relcatHead;
    check=relRec.getHeader(&relcatHead);
    if(check!=SUCCESS){
        return check;
    }

    /* Decrement the numEntries in the header of the block corresponding to the
       relation catalog entry and set it back */
    relcatHead.numEntries-=1;
    check=relRec.setHeader(&relcatHead);
    if(check!=SUCCESS){
        return check;
    }

    /* Get the slotmap in relation catalog, update it by marking the slot as
       free(SLOT_UNOCCUPIED) and set it back. */
    unsigned char slotMap[relcatHead.numSlots];
    check=relRec.getSlotMap(slotMap);
    if(check!=SUCCESS){
        return check;
    }
    slotMap[rec_id.slot] = SLOT_UNOCCUPIED;
    check=relRec.setSlotMap(slotMap);
    if(check!=SUCCESS){
        return check;
    }

    /*** Updating the Relation Cache Table ***/
    /** Update relation catalog record entry (number of records in relation
        catalog is decreased by 1) **/
    // Get the entry corresponding to relation catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)
    RelCatEntry relEntry;
    RelCacheTable::getRelCatEntry(RELCAT_RELID,&relEntry);
    relEntry.numRecs-=1;
    RelCacheTable::setRelCatEntry(RELCAT_RELID,&relEntry);

    /** Update attribute catalog entry (number of records in attribute catalog
        is decreased by numberOfAttributesDeleted) **/
    // i.e., #Records = #Records - numberOfAttributesDeleted
    RelCatEntry atbEntry;
    RelCacheTable::getRelCatEntry(ATTRCAT_RELID,&atbEntry);
    atbEntry.numRecs-=numberOfAttributesDeleted;
    RelCacheTable::setRelCatEntry(ATTRCAT_RELID,&atbEntry);
    

    // Get the entry corresponding to attribute catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)


    return SUCCESS;
}

/*
NOTE: the caller is expected to allocate space for the argument `record` based
      on the size of the relation. This function will only copy the result of
      the projection onto the array pointed to by the argument.
*/
int BlockAccess::project(int relId, Attribute *record) {
    // get the previous search index of the relation relId from the relation
    // cache (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId,&prevRecId);

    // declare block and slot which will be used to store the record id of the
    // slot we need to check.
    int block, slot;

    /* if the current search index record is invalid(i.e. = {-1, -1})
       (this only happens when the caller reset the search index)
    */

    
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (new project operation. start from beginning)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        
        RelCatEntry relEntry;
        RelCacheTable::getRelCatEntry(relId,&relEntry);
        block=relEntry.firstBlk;
        slot=0;

        // block = first record block of the relation
        // slot = 0
    }
    else
    {
        // (a project/search operation is already in progress)
        block=prevRecId.block;
        slot=prevRecId.slot+1;

        // block = previous search index's block
        // slot = previous search index's slot + 1
    }


    // The following code finds the next record of the relation
    /* Start from the record id (block, slot) and iterate over the remaining
       records of the relation */
    while (block != -1)
    {
        // create a RecBuffer object for block (using appropriate constructor!)
        RecBuffer currentBlockBuffer (block);

        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function
		HeadInfo currentBlockHeader;
		currentBlockBuffer.getHeader(&currentBlockHeader);

		unsigned char slotmap [currentBlockHeader.numSlots];
		currentBlockBuffer.getSlotMap(slotmap);

        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function

        if(slot>=currentBlockHeader.numSlots)
        {   
            block=currentBlockHeader.rblock;
            slot=0;
            // (no more slots in this block)
            // update block = right block of block
            // update slot = 0
            // (NOTE: if this is the last block, rblock would be -1. this would
            //        set block = -1 and fail the loop condition )
        }
        else if (slotmap[slot]==SLOT_UNOCCUPIED)
        { // (i.e slot-th entry in slotMap contains SLOT_UNOCCUPIED)
            slot+=1;
            // increment slot
        }
        else {
            // (the next occupied slot / record has been found)
            break;
        }
    }

    if (block == -1){
        // (a record was not found. all records exhausted)
        return E_NOTFOUND;
    }

    // declare nextRecId to store the RecId of the record found
    RecId nextRecId{block, slot};

    // set the search index to nextRecId using RelCacheTable::setSearchIndex
    RelCacheTable::setSearchIndex(relId,&nextRecId);
    /* Copy the record with record id (nextRecId) to the record buffer (record)
       For this Instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
   RecBuffer recordBuf(nextRecId.block);
   recordBuf.getRecord(record,nextRecId.slot);
   

    return SUCCESS;
}