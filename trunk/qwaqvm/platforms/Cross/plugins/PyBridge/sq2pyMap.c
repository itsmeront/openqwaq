/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-2011, Teleplace, Inc., All Rights Reserved
 *
 * Redistributions in source code form must reproduce the above
 * copyright and this condition.
 *
 * The contents of this file are subject to the GNU General Public
 * License, Version 2 (the "License"); you may not use this file
 * except in compliance with the License. A copy of the License is
 * available at http://www.opensource.org/licenses/gpl-2.0.php.
 *
 */

 /*
 
  sq2pyMap.c: Sq2PyMap registry implementation

*/
#include "Python.h"

#include "sqVirtualMachine.h"
#include "sqConfig.h"
#include "sqPlatformSpecific.h"

#include "PyBridge.h"
#include "sq2pyMap.h"

/* hashBitsOf: Extract hash bits from a Squeak OOP.
   Returns the hash value of the object.
   Arguments:
     obj: The OOP for which to compute the hash bits.
   NOTE: hashBitsOf() should really be exposed by the
   interpreter proxy -- this is just a cheesy workaround.
*/
int hashBitsOf(OOP obj) {
  if(vm->isIntegerObject(obj)) return vm->integerValueOf(obj);
  /* copied from Interpreter>>hashBitsOf: */
  return ((*(int*)obj) >> 17) & 0xFFF;
}

/* sq2pyMapNew: Create a new sq2py map.
   Returns the new map or NULL if not successful.
   Arguments:
     nItems: The initial size of the map.
*/
Sq2PyMap *sq2pyMapNew(int nItems) {
  Sq2PyMap *map;
  map = calloc(1, sizeof(Sq2PyMap));
  map->max = nItems;
  map->sqObj = vm->instantiateClassindexableSize(vm->classArray(), map->max);
  map->pyObj = calloc(map->max, sizeof(PyObject*));
  vm->addGCRoot(&map->sqObj);
  return map;
}

/* sq2pyMapFree: Free the given sq2py map.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The map to be freed.
*/
int sq2pyMapFree(Sq2PyMap *map) {
  free(map->pyObj);
  vm->removeGCRoot(&map->sqObj);
  free(map);
  return 1;
}

/* sq2pyMapIndexOf: Find the slot index for a Squeak OOP in the given map.
   Returns the index for the slot or -1 on error.
   Arguments:
     map: The Sq2PyMap to use.
     sqObj: The Squeak OOP to find.
*/
int sq2pyMapIndexOf(Sq2PyMap *map, OOP sqObj) {
  unsigned int hash = hashBitsOf(sqObj);
  int finish = vm->slotSizeOf(map->sqObj);
  int start  = hash % finish;
  OOP *data = vm->firstIndexableField(map->sqObj);
  OOP nilOop = vm->nilObject();
  int i;

  /* search from start to finish */
  for(i=start; i<finish; i++) {
    OOP item = data[i];
    if(item == nilOop || item == sqObj) return i;
  }

  /* search from 0 to start */
  for(i=0; i<start; i++) {
    OOP item = data[i];
    if(item == nilOop || item == sqObj) return i;
  }
  pyErr = "sq2pyIndexOf: Object not found and no empty slot";
  return -1; /* not found and no empty slot */
}

/* sq2pyMapFind: Find the PyObject for an SqObject.
   Returns the PyObject or NULL if not present.
   Arguments:
     map: The Sq2PyMap to search.
     sqObj: The Squeak object to find.
*/
PyObject *sq2pyMapFind(Sq2PyMap *map, OOP sqObj) {
  int index = sq2pyMapIndexOf(map, sqObj);
  if(index < 0) return NULL;
  if(vm->fetchPointerofObject(index, map->sqObj) == sqObj)
    return map->pyObj[index];
  return NULL;
}

/* sq2pyMapGrow: Grow the map to contain more elements.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The map to grow.
*/
int sq2pyMapGrow(Sq2PyMap *oldMap) {
  if(oldMap->size*2 > oldMap->max) {
    int i;
    Sq2PyMap *tmpMap = sq2pyMapNew(oldMap->max*2-1);
    OOP      *sqOldData = vm->firstIndexableField(oldMap->sqObj);
    OOP      *sqNewData = vm->firstIndexableField(tmpMap->sqObj);
    PyObject **pyOldData = oldMap->pyObj;
    PyObject **pyNewData = tmpMap->pyObj;
    OOP       nilOop = vm->nilObject();
    /* SUBTLETY WARNING: 
       The only reason why we can use direct array access for
       sqNewData[] is that we have just allocated it (via py2sqMapNew())
       and therefore all values stored will -by definition- be "older"
       than the tmpMap. If that were different we would have to use
       storePointerofObjectwithValue so that the VM can record the object
       as root if necessary. */
    for(i=0; i < oldMap->max; i++) {
      OOP       sqObj = sqOldData[i];
      PyObject *pyObj = pyOldData[i];
      if(sqObj != nilOop) {
	int index = sq2pyMapIndexOf(tmpMap, sqObj);
	sqNewData[index] = sqObj;
	pyNewData[index] = pyObj;
      }
    }
    /* now swap the contents of oldMap and tmpMap */
    {
      OOP sqOldObj = oldMap->sqObj;
      PyObject **pyOldObj = oldMap->pyObj;
      int oldSize = oldMap->size;
      int oldMax = oldMap->max;

      oldMap->size = tmpMap->size;
      oldMap->max = tmpMap->max;
      oldMap->pyObj = tmpMap->pyObj;
      oldMap->sqObj = tmpMap->sqObj;

      tmpMap->size = oldSize;
      tmpMap->max = oldMax;
      tmpMap->pyObj = pyOldObj;
      tmpMap->sqObj = sqOldObj;
    }
    sq2pyMapFree(tmpMap);
  }
  return 1;
}

/* sq2pyMapAdd: Add an SqObject->PyObject association.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The Sq2PyMap to add to.
     sqObj: The Squeak OOP to use.
     pyObj: The PyObject reference to use.
*/
int sq2pyMapAdd(Sq2PyMap *map, OOP sqObj, PyObject *pyObj) {
  int index = sq2pyMapIndexOf(map, sqObj);
  if(index < 0) return 0;
  if(vm->fetchPointerofObject(index, map->sqObj) == vm->nilObject()) {
    /* new entry in map */
    map->size++;
    vm->storePointerofObjectwithValue(index, map->sqObj, sqObj);
  } else {
    /* object already in map; decrement PyObj refcount */
    PyObject *old = map->pyObj[index];
    Py_DECREF(old);
  }
  /* pyObj to map */
  Py_INCREF(pyObj);
  map->pyObj[index] = pyObj;
  return 1;
}

/* sq2pyUnmap: Undo the mapping of some Squeak OOP.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The Sq2PyMap to remove the association from.
     sqObj: The OOP for the association to remove.
*/
int sq2pyUnmap(Sq2PyMap *map, OOP sqObj) {
  int oldIndex, newIndex;
  int index = sq2pyMapIndexOf(map, sqObj);
  OOP      *sqData = vm->firstIndexableField(map->sqObj);
  OOP       nilOop = vm->nilObject();
  PyObject **pyData = map->pyObj;
  /* SUBTLETY WARNING:
     The usage of sqData[] only works because we're only changing
     pointers inside it, meaning that we do not need to guard root
     stores. If that were different we'd have to explicitly use
     storePointerofObjectwithValue to guard those stores.
  */

  if(index == -1) return 0;

  if(sqData[index] != sqObj) {
    /* sqObj wasn't in the map to begin with */
    pyErr = "sq2pyUnmap: sqOop not present in map";
    return 0;
  }

  map->size--;
  sqData[index] = nilOop;
  Py_DECREF(pyData[index]);
  pyData[index] = NULL;

  /* fix collisions starting at index */
  oldIndex = (index+1) % map->max;
  while(sqData[oldIndex] != nilOop) {
    newIndex = sq2pyMapIndexOf(map, sqData[oldIndex]);
    if(newIndex != oldIndex) {
      sqData[newIndex] = sqData[oldIndex];
      sqData[oldIndex] = nilOop;
      pyData[newIndex] = pyData[oldIndex];
      pyData[oldIndex] = NULL;
    }
    oldIndex = (oldIndex+1) % map->max;
  }
  return 1;
}

