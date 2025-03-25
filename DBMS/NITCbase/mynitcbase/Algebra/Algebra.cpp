#include "Algebra.h"
#include <cstring>
#include <iostream>
using namespace std;

/* used to select all the records that satisfy a condition.
the arguments of the function are
- srcRel - the source relation we want to select from
- targetRel - the relation we want to select into. (ignore for now)
- attr - the attribute that the condition is checking
- op - the operator of the condition
- strVal - the value that we want to compare against (represented as a string)

*/

bool isNumber(char *str)
{
  int len;
  float ignore;
  /*
    sscanf returns the number of elements read, so if there is no float matching
    the first %f, ret will be 0, else it'll be 1

    %n gets the number of characters read. this scanf sequence will read the
    first float ignoring all the whitespace before and after. and the number of
    characters read that far will be stored in len. if len == strlen(str), then
    the string only contains a float with/without whitespace. else, there's
    other characters.
  */
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
{
  // get the srcRel's rel-id (let it be srcRelid), using OpenRelTable::getRelId()
  // if srcRel is not open in open relation table, return E_RELNOTOPEN
  relId srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN)
  {
    return E_RELNOTOPEN;
  }

  AttrCatEntry attrCatEntry;
  int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
  if (ret == E_ATTRNOTEXIST)
  {
    return E_ATTRNOTEXIST;
  }

  // get the attr-cat entry for attr, using AttrCacheTable::getAttrCatEntry()
  // if getAttrcatEntry() call fails return E_ATTRNOTEXIST

  /*** Convert strVal to an attribute of data type NUMBER or STRING ***/

  Attribute attrVal;
  int type = attrCatEntry.attrType;

  if (type == NUMBER)
  {
    // if the input argument strVal can be converted to a number
    // (check this using isNumber() function)
    if (isNumber(strVal))
    {
      // convert strVal to double and store it at attrVal.nVal using atof()
      attrVal.nVal = atof(strVal);
    }
    else
    {
      return E_ATTRTYPEMISMATCH;
    }
  }
  else if (type == STRING)
  {
    // copy strVal to attrVal.sVal
    strcpy(attrVal.sVal, strVal);
  }

  /*** Creating and opening the target relation ***/
  // Prepare arguments for createRel() in the following way:
  // get RelcatEntry of srcRel using RelCacheTable::getRelCatEntry()
  RelCacheTable::resetSearchIndex(srcRelId);
  RelCatEntry relEntry;
  RelCacheTable::getRelCatEntry(srcRelId, &relEntry);
  int src_nAttrs = relEntry.numAttrs;

  /* let attr_names[src_nAttrs][ATTR_SIZE] be a 2D array of type char
      (will store the attribute names of rel). */
  // let attr_types[src_nAttrs] be an array of type int
  char attr_names[src_nAttrs][ATTR_SIZE];
  int attr_types[src_nAttrs];

  /*iterate through 0 to src_nAttrs-1 :
      get the i'th attribute's AttrCatEntry using AttrCacheTable::getAttrCatEntry()
      fill the attr_names, attr_types arrays that we declared with the entries
      of corresponding attributes
  */
  for (int i = 0; i < src_nAttrs; i++)
  {
    AttrCatEntry tempAtbEntry;
    AttrCacheTable::getAttrCatEntry(srcRelId, i, &tempAtbEntry);

    strcpy(attr_names[i], tempAtbEntry.attrName);
    attr_types[i] = tempAtbEntry.attrType;
  }

  ret = Schema::createRel(targetRel, src_nAttrs, attr_names, attr_types);
  if (ret != SUCCESS)
  {
    return ret;
  }

  relId targetRelId = OpenRelTable::openRel(targetRel);
  if (targetRelId < 0 || targetRelId >= MAX_OPEN)
  {
    Schema::deleteRel(targetRel);
    return targetRelId;
  }

  /* Create the relation for target relation by calling Schema::createRel()
     by providing appropriate arguments */
  // if the createRel returns an error code, then return that value.

  /* Open the newly created target relation by calling OpenRelTable::openRel()
     method and store the target relid */
  /* If opening fails, delete the target relation by calling Schema::deleteRel()
     and return the error value returned from openRel() */

  /*** Selecting and inserting records into the target relation ***/
  /* Before calling the search function, reset the search to start from the
     first using RelCacheTable::resetSearchIndex() */

  Attribute record[src_nAttrs];

  /*
      The BlockAccess::search() function can either do a linearSearch or
      a B+ tree search. Hence, reset the search index of the relation in the
      relation cache using RelCacheTable::resetSearchIndex().
      Also, reset the search index in the attribute cache for the select
      condition attribute with name given by the argument `attr`. Use
      AttrCacheTable::resetSearchIndex().
      Both these calls are necessary to ensure that search begins from the
      first record.
  */

  RelCacheTable::resetSearchIndex(srcRelId);
  AttrCacheTable::resetSearchIndex(srcRelId, attr); // AFTER WHEN B++

  // read every record that satisfies the condition by repeatedly calling
  // BlockAccess::search() until there are no more records to be read

  StaticBuffer::numCompares = 0;
  while (BlockAccess::search(srcRelId, record, attr, attrVal, op) == SUCCESS)
  {

    // ret = BlockAccess::insert(targetRelId, record);
    ret = BlockAccess::insert(targetRelId, record);

    if (ret != SUCCESS)
    {
      Schema::closeRel(targetRel);
      Schema::deleteRel(targetRel);
      return ret;
    }
    // if (insert fails) {
    //     close the targetrel(by calling Schema::closeRel(targetrel))
    //     delete targetrel (by calling Schema::deleteRel(targetrel))
    //     return ret;
    // }
  }
  printf("Number of Comparisons DONE:%d\n", StaticBuffer::numCompares);
  // Close the targetRel by calling closeRel() method of schema layer
  Schema::closeRel(targetRel);

  return SUCCESS;

  // return SUCCESS.
}

// will return if a string can be parsed as a floating point number

int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE])
{
  // if relName is equal to "RELATIONCAT" or "ATTRIBUTECAT"
  // return E_NOTPERMITTED;
  if (strcmp(RELCAT_RELNAME, relName) == 0 || strcmp(ATTRCAT_RELNAME, relName) == 0)
  {
    return E_NOTPERMITTED;
  }

  // get the relation's rel-id using OpenRelTable::getRelId() method
  int relId = OpenRelTable::getRelId(relName);
  if (relId == E_RELNOTOPEN)
  {
    return E_RELNOTOPEN;
  }

  // if relation is not open in open relation table, return E_RELNOTOPEN
  // (check if the value returned from getRelId function call = E_RELNOTOPEN)
  // get the relation catalog entry from relation cache
  // (use RelCacheTable::getRelCatEntry() of Cache Layer)

  /* if relCatEntry.numAttrs != numberOfAttributes in relation,
     return E_NATTRMISMATCH */

  RelCatEntry relEntry;
  RelCacheTable::getRelCatEntry(relId, &relEntry);
  if (relEntry.numAttrs != nAttrs)
  {
    return E_NATTRMISMATCH;
  }

  // let recordValues[numberOfAttributes] be an array of type union Attribute

  Attribute recVals[nAttrs];

  /*
      Converting 2D char array of record values to Attribute array recordValues
   */
  // iterate through 0 to nAttrs-1: (let i be the iterator)
  for (int i = 0; i < nAttrs; i++)
  {
    // get the attr-cat entry for the i'th attribute from the attr-cache
    // (use AttrCacheTable::getAttrCatEntry())
    // let type = attrCatEntry.attrType;

    AttrCatEntry attrEntry;
    AttrCacheTable::getAttrCatEntry(relId, i, &attrEntry);
    int type = attrEntry.attrType;
    if (type == NUMBER)
    {
      // if the char array record[i] can be converted to a number
      // (check this using isNumber() function)

      if (isNumber(record[i]))
      {
        /* convert the char array to numeral and store it at recordValues[i].nVal using atof() */
        recVals[i].nVal = atof(record[i]);
      }
      else
      {
        return E_ATTRTYPEMISMATCH;
      }
    }
    else if (type == STRING)
    {
      // copy record[i] to recordValues[i].sVal
      strcpy(recVals[i].sVal, record[i]);
    }
  }

  // insert the record by calling BlockAccess::insert() function
  // let retVal denote the return value of insert call
  
  int retVal = BlockAccess::insert(relId, recVals);
  // printf("NumOfBlocks:%d\n",StaticBuffer::NumOfBlock);

  return retVal;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE])
{

  int srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN)
  {
    return E_RELNOTOPEN;
  }

  // if srcRel is not open in open relation table, return E_RELNOTOPEN

  // get RelCatEntry of srcRel using RelCacheTable::getRelCatEntry()

  // get the no. of attributes present in relation from the fetched RelCatEntry.

  // attrNames and attrTypes will be used to store the attribute names
  // and types of the source relation respectively

  RelCatEntry relEntry;
  RelCacheTable::getRelCatEntry(srcRelId, &relEntry);
  int numAttrs = relEntry.numAttrs;

  char attrNames[numAttrs][ATTR_SIZE];
  int attrTypes[numAttrs];

  /*iterate through every attribute of the source relation :
      - get the AttributeCat entry of the attribute with offset.
        (using AttrCacheTable::getAttrCatEntry())
      - fill the arrays `attrNames` and `attrTypes` that we declared earlier
        with the data about each attribute
  */
  for (int i = 0; i < numAttrs; i++)
  {
    AttrCatEntry attrEntry;
    AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrEntry);
    strcpy(attrNames[i], attrEntry.attrName);
    attrTypes[i] = attrEntry.attrType;
  }

  /*** Creating and opening the target relation ***/

  // Create a relation for target relation by calling Schema::createRel()

  // if the createRel returns an error code, then return that value.

  // Open the newly created target relation by calling OpenRelTable::openRel()
  // and get the target relid

  // If opening fails, delete the target relation by calling Schema::deleteRel() of
  // return the error value returned from openRel().
  int check = Schema::createRel(targetRel, numAttrs, attrNames, attrTypes);
  if (check != SUCCESS)
  {
    return check;
  }
  relId targetRelId = OpenRelTable::openRel(targetRel);
  if (targetRelId < 0 || targetRelId >= MAX_OPEN)
  {
    Schema::deleteRel(targetRel);
    return check;
  }

  /*** Inserting projected records into the target relation ***/

  // Take care to reset the searchIndex before calling the project function
  // using RelCacheTable::resetSearchIndex()
  RelCacheTable::resetSearchIndex(srcRelId);
  Attribute record[numAttrs];

  while (BlockAccess::project(srcRelId, record) == SUCCESS)
  {
    // record will contain the next record

    // ret = BlockAccess::insert(targetRelId, proj_record);
    check = BlockAccess::insert(targetRelId, record);

    if (check != SUCCESS)
    {
      Schema::closeRel(targetRel);
      Schema::deleteRel(targetRel);
      return check;
      // close the targetrel by calling Schema::closeRel()
      // delete targetrel by calling Schema::deleteRel()
      // return ret;
    }
  }

  Schema::closeRel(targetRel);
  // Close the targetRel by calling Schema::closeRel()

  return SUCCESS;
  // return SUCCESS.
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE])
{

  int srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN)
  {
    return E_RELNOTOPEN;
  }

  // if srcRel is not open in open relation table, return E_RELNOTOPEN

  // get RelCatEntry of srcRel using RelCacheTable::getRelCatEntry()
  // get the no. of attributes present in relation from the fetched RelCatEntry.
  RelCatEntry relEntry;
  RelCacheTable::getRelCatEntry(srcRelId, &relEntry);
  int src_nAttrs = relEntry.numAttrs;

  // declare attr_offset[tar_nAttrs] an array of type int.
  // where i-th entry will store the offset in a record of srcRel for the
  // i-th attribute in the target relation.

  // let attr_types[tar_nAttrs] be an array of type int.
  // where i-th entry will store the type of the i-th attribute in the
  // target relation.

  /*** Checking if attributes of target are present in the source relation
       and storing its offsets and types ***/

  /*iterate through 0 to tar_nAttrs-1 :
      - get the attribute catalog entry of the attribute with name tar_attrs[i].
      - if the attribute is not found return E_ATTRNOTEXIST
      - fill the attr_offset, attr_types arrays of target relation from the
        corresponding attribute catalog entries of source relation
  */
  int attr_offset[tar_nAttrs];
  int attr_types[tar_nAttrs];

  for (int i = 0; i < tar_nAttrs; i++)
  {
    AttrCatEntry attrEntry;
    int check = AttrCacheTable::getAttrCatEntry(srcRelId, tar_Attrs[i], &attrEntry);
    if (check == E_ATTRNOTEXIST)
    {
      return E_ATTRNOTEXIST;
    }
    attr_offset[i] = attrEntry.offset;
    attr_types[i] = attrEntry.attrType;
  }

  /*** Creating and opening the target relation ***/

  // Create a relation for target relation by calling Schema::createRel()

  // if the createRel returns an error code, then return that value.

  // Open the newly created target relation by calling OpenRelTable::openRel()
  // and get the target relid

  // If opening fails, delete the target relation by calling Schema::deleteRel()
  // and return the error value from openRel()
  int check = Schema::createRel(targetRel, tar_nAttrs, tar_Attrs, attr_types);
  if (check != SUCCESS)
  {
    return check;
  }

  relId targetRelId = OpenRelTable::openRel(targetRel);
  if (targetRelId < 0 || targetRelId >= MAX_OPEN)
  {
    Schema::deleteRel(targetRel);
    return targetRelId;
  }

  /*** Inserting projected records into the target relation ***/

  // Take care to reset the searchIndex before calling the project function
  // using RelCacheTable::resetSearchIndex()
  RelCacheTable::resetSearchIndex(srcRelId);
  Attribute record[src_nAttrs];

  while (BlockAccess::project(srcRelId, record) == SUCCESS)
  {
    // the variable `record` will contain the next record

    Attribute proj_record[tar_nAttrs];

    // iterate through 0 to tar_attrs-1:
    //     proj_record[attr_iter] = record[attr_offset[attr_iter]]
    for (int i = 0; i < tar_nAttrs; i++)
    {
      proj_record[i] = record[attr_offset[i]];
    }

    check = BlockAccess::insert(targetRelId, proj_record);

    // ret = BlockAccess::insert(targetRelId, proj_record);

    if (check != SUCCESS)
    {
      Schema::closeRel(targetRel);
      Schema::deleteRel(targetRel);
      return check;
      // close the targetrel by calling Schema::closeRel()
      // delete targetrel by calling Schema::deleteRel()
      // return ret;
    }
  }

  Schema::closeRel(targetRel);

  return SUCCESS;
  // Close the targetRel by calling Schema::closeRel()

  // return SUCCESS.
}

int Algebra::join(char srcRelation1[ATTR_SIZE], char srcRelation2[ATTR_SIZE], char targetRelation[ATTR_SIZE], char attribute1[ATTR_SIZE], char attribute2[ATTR_SIZE])
{

  // get the srcRelation1's rel-id using OpenRelTable::getRelId() method
  int srcRelid1 = OpenRelTable::getRelId(srcRelation1);
  int srcRelid2 = OpenRelTable::getRelId(srcRelation2);

  // get the srcRelation2's rel-id using OpenRelTable::getRelId() method
  if (srcRelid1 == E_RELNOTOPEN || srcRelid2 == E_RELNOTOPEN)
  {
    return E_RELNOTOPEN;
  }
  // if either of the two source relations is not open
  //     return E_RELNOTOPEN

  AttrCatEntry attrCatEntry1, attrCatEntry2;
  // get the attribute catalog entries for the following from the attribute cache
  // (using AttrCacheTable::getAttrCatEntry())
  // - attrCatEntry1 = attribute1 of srcRelation1
  // - attrCatEntry2 = attribute2 of srcRelation2
  int ret1 = AttrCacheTable::getAttrCatEntry(srcRelid1, attribute1, &attrCatEntry1);
  int ret2 = AttrCacheTable::getAttrCatEntry(srcRelid2, attribute2, &attrCatEntry2);

  // if attribute1 is not present in srcRelation1 or attribute2 is not
  // present in srcRelation2 (getAttrCatEntry() returned E_ATTRNOTEXIST)
  //     return E_ATTRNOTEXIST.
  if (ret1 == E_ATTRNOTEXIST || ret2 == E_ATTRNOTEXIST)
  {
    return E_ATTRNOTEXIST;
  }

  // if attribute1 and attribute2 are of different types return E_ATTRTYPEMISMATCH
  if (attrCatEntry1.attrType != attrCatEntry2.attrType)
  {
    return E_ATTRTYPEMISMATCH;
  }

  // iterate through all the attributes in both the source relations and check if
  // there are any other pair of attributes other than join attributes
  // (i.e. attribute1 and attribute2) with duplicate names in srcRelation1 and
  // srcRelation2 (use AttrCacheTable::getAttrCatEntry())
  // If yes, return E_DUPLICATEATTR

  // get the relation catalog entries for the relations from the relation cache
  // (use RelCacheTable::getRelCatEntry() function)
  RelCatEntry relEntry1;
  RelCatEntry relEntry2;

  RelCacheTable::getRelCatEntry(srcRelid1, &relEntry1);
  RelCacheTable::getRelCatEntry(srcRelid2, &relEntry2);

  int numOfAttributes1 = relEntry1.numAttrs;
  int numOfAttributes2 = relEntry2.numAttrs;

  for (int i = 0; i < numOfAttributes1; i++)
  {
    AttrCatEntry attrCatBuf1;
    AttrCacheTable::getAttrCatEntry(srcRelid1, i, &attrCatBuf1);

    for (int j = 0; j < numOfAttributes2; j++)
    {
      AttrCatEntry attrCatBuf2;
      AttrCacheTable::getAttrCatEntry(srcRelid2, j, &attrCatBuf2);

      if (i == attrCatEntry1.offset && j == attrCatEntry2.offset)
        continue;

      if (strcmp(attrCatBuf1.attrName, attrCatBuf2.attrName) == 0)
        return E_DUPLICATEATTR;
    }
  }

  // if rel2 does not have an index on attr2
  //     create it using BPlusTree:bPlusCreate()
  //     if call fails, return the appropriate error code
  //     (if your implementation is correct, the only error code that will
  //      be returned here is E_DISKFULL)
  if (attrCatEntry2.rootBlock == -1)
  {
    int check = BPlusTree::bPlusCreate(srcRelid2, attribute2);
    if (check != SUCCESS)
    {
      return check;
    }
  }

  int numOfAttributesInTarget = numOfAttributes1 + numOfAttributes2 - 1;
  // Note: The target relation has number of attributes one less than
  // nAttrs1+nAttrs2 (Why?)

  // declare the following arrays to store the details of the target relation
  char targetRelAttrNames[numOfAttributesInTarget][ATTR_SIZE];
  int targetRelAttrTypes[numOfAttributesInTarget];

  // iterate through all the attributes in both the source relations and
  // update targetRelAttrNames[],targetRelAttrTypes[] arrays excluding attribute2
  // in srcRelation2 (use AttrCacheTable::getAttrCatEntry())
  int x = 0;
  for (int i = 0; i < numOfAttributes1; i++)
  {
    AttrCatEntry tempEn1;
    AttrCacheTable::getAttrCatEntry(srcRelid1, i, &tempEn1);
    targetRelAttrTypes[x] = tempEn1.attrType;
    strcpy(targetRelAttrNames[x], tempEn1.attrName);
    x++;
  }
  
  for (int i = 0; i < numOfAttributes2; i++)
  {
    AttrCatEntry tempEn2;
    AttrCacheTable::getAttrCatEntry(srcRelid2, i, &tempEn2);
    if (tempEn2.offset==attrCatEntry2.offset)
    {
      continue;
    }
    targetRelAttrTypes[x] = tempEn2.attrType;
    strcpy(targetRelAttrNames[x], tempEn2.attrName);
    x++;
  }

  int ret = Schema::createRel(targetRelation, numOfAttributesInTarget, targetRelAttrNames, targetRelAttrTypes);
  if (ret != SUCCESS)
  {
    return ret;
  }

  int targetRelid = OpenRelTable::openRel(targetRelation);

  // create the target relation using the Schema::createRel() function

  // if createRel() returns an error, return that error

  // Open the targetRelation using OpenRelTable::openRel()

  // if openRel() fails (No free entries left in the Open Relation Table)
  if (targetRelid < 0 || targetRelid >= MAX_OPEN)
  {
    // delete target relation by calling Schema::deleteRel()
    Schema::deleteRel(targetRelation);
    return targetRelid;
    // return the error code
  }

  Attribute record1[numOfAttributes1];
  Attribute record2[numOfAttributes2];
  Attribute targetRecord[numOfAttributesInTarget];

  // this loop is to get every record of the srcRelation1 one by one
  RelCacheTable::resetSearchIndex(srcRelid1);
  AttrCacheTable::resetSearchIndex(srcRelid1, attribute1);
  while (BlockAccess::project(srcRelid1, record1) == SUCCESS)
  {

    // reset the search index of `srcRelation2` in the relation cache
    // using RelCacheTable::resetSearchIndex()
    RelCacheTable::resetSearchIndex(srcRelid2);
    AttrCacheTable::resetSearchIndex(srcRelid2, attribute2);

    // reset the search index of `attribute2` in the attribute cache
    // using AttrCacheTable::resetSearchIndex()

    // this loop is to get every record of the srcRelation2 which satisfies
    // the following condition:
    // record1.attribute1 = record2.attribute2 (i.e. Equi-Join condition)
    while (BlockAccess::search(srcRelid2, record2, attribute2, record1[attrCatEntry1.offset], EQ) == SUCCESS)
    {

      // copy srcRelation1's and srcRelation2's attribute values(except
      // for attribute2 in rel2) from record1 and record2 to targetRecord
      int x = 0;
      for (int i = 0; i < numOfAttributes1; i++)
      {
        targetRecord[x] = record1[i];
        x++;
      }
      
      for (int i = 0; i < numOfAttributes2; i++)
      {
        if (i == attrCatEntry2.offset)
        {
          continue;
        }
        targetRecord[x] = record2[i];
        x++;
      }

      int retr = BlockAccess::insert(targetRelid, targetRecord);

      // insert the current record into the target relation by calling
      // BlockAccess::insert()

      if (retr != SUCCESS)
      {

        // close the target relation by calling OpenRelTable::closeRel()
        // delete targetRelation (by calling Schema::deleteRel())
        OpenRelTable::closeRel(targetRelid);
        Schema::deleteRel(targetRelation);
        return E_DISKFULL;
      }
    }
  }

  // close the target relation by calling OpenRelTable::closeRel()
  OpenRelTable::closeRel(targetRelid);
  return SUCCESS;
}


