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

  py2sqMap.c: Py2SqMap registry implementation

*/
#include "Python.h"

#include "sqVirtualMachine.h"
#include "sqConfig.h"
/* #include "sqPlatformSpecific.h" */

#include "PyBridge.h"
#include "py2sqMap.h"

/* py2sqMapNew: Create a new py2sq map.
   Returns the new map or NULL if not successful.
   Arguments:
     nItems: The initial size of the map.
*/
Py2SqMap *py2sqMapNew(int nItems) {
  Py2SqMap *map;
  map = calloc(1, sizeof(Py2SqMap));
  map->max = nItems;
  map->pyObj = calloc(map->max, sizeof(PyObject*));
  map->sqObj = vm->instantiateClassindexableSize(vm->classArray(), map->max);
  vm->addGCRoot(&map->sqObj);
  return map;
}

/* py2sqMapFree: Free the given py2sq map.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The map to be freed.
*/
int py2sqMapFree(Py2SqMap *map) {
  free(map->pyObj);
  vm->removeGCRoot(&map->sqObj);
  free(map);
  return 1;
}

/* py2sqMapIndexOf: Find the slot index for a PyObject in the given map.
   Returns the index for the slot or -1 on error.
   Arguments:
     map: The Py2SqMap to use.
     pyObj: The PyObject to find.
*/
int py2sqMapIndexOf(Py2SqMap *map, PyObject *pyObj) {
  unsigned int hash = (unsigned int) pyObj;
  int finish = map->max;
  int start  = hash % finish;
  PyObject **data = map->pyObj;
  int i;

  /* search from start to finish */
  for(i=start; i<finish; i++) {
    PyObject *item = data[i];
    if(item == NULL || item == pyObj) return i;
  }

  /* search from 0 to start */
  for(i=0; i<start; i++) {
    PyObject *item = data[i];
    if(item == NULL || item == pyObj) return i;
  }
  pyErr = "py2sqIndexOf: Object not found and no empty slot";
  return -1; /* not found and no empty slot */
}

/* py2sqMapFind: Find the Sqeak OOP for a PyObject.
   Returns the OOP or zero if not present.
   Arguments:
     map: The Py2SqMap to search.
     pyObj: The Squeak object to find.
*/
OOP py2sqMapFind(Py2SqMap *map, PyObject *pyObj) {
  int index = py2sqMapIndexOf(map, pyObj);
  if(index < 0) return 0;
  if(map->pyObj[index] == pyObj)
    return vm->fetchPointerofObject(index, map->sqObj);
  return 0;
}

/* py2sqMapGrow: Grow the map to contain more elements.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The map to grow.
*/
int py2sqMapGrow(Py2SqMap *oldMap) {
  if(oldMap->size*2 > oldMap->max) {
    int i;
    Py2SqMap *tmpMap = py2sqMapNew(oldMap->max*2-1);
    PyObject **pyOldData = oldMap->pyObj;
    PyObject **pyNewData = tmpMap->pyObj;
    OOP      *sqOldData = vm->firstIndexableField(oldMap->sqObj);
    OOP      *sqNewData = vm->firstIndexableField(tmpMap->sqObj);
    /* SUBTLETY WARNING: 
       The only reason why we can use direct array access for
       sqNewData[] is that we have just allocated it (via py2sqMapNew())
       and therefore all values stored will -by definition- be "older"
       than the tmpMap. If that were different we would have to use
       storePointerofObjectwithValue: such that the VM can record the object
       as root if necessary. */
    for(i=0; i < oldMap->max; i++) {
      PyObject *pyObj = pyOldData[i];
      OOP       sqObj = sqOldData[i];
      if(pyObj) {
	int index = py2sqMapIndexOf(tmpMap, pyObj);
	pyNewData[index] = pyObj;
	sqNewData[index] = sqObj;
      }
    }
    /* now swap the contents of oldMap and tmpMap */
    {
      PyObject **pyOldObj = oldMap->pyObj;
      OOP sqOldObj = oldMap->sqObj;
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
    py2sqMapFree(tmpMap);
  }
  return 1;
}

/* py2sqMapAdd: Add a PyObject->Squeak OOP association.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The Sq2PyMap to add to.
     pyObj: The PyObject reference to use.
     sqObj: The Squeak OOP to use.
*/
int py2sqMapAdd(Py2SqMap *map, PyObject *pyObj, OOP sqObj) {
  int index = py2sqMapIndexOf(map, pyObj);
  if(index < 0) return 0;
  if(map->pyObj[index] == NULL) {
    /* this is a new entry in the map */
    Py_INCREF(pyObj);
    map->size++;
    map->pyObj[index] = pyObj;
  }
  vm->storePointerofObjectwithValue(index, map->sqObj, sqObj);
  return 1;
}

/* py2sqUnmap: Undo the mapping of some PyObject.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The Py2SqMap to remove the association from.
     pyObj: The PyObject for the association to remove.
*/
int py2sqUnmap(Py2SqMap *map, PyObject *pyObj) {
  int oldIndex, newIndex;
  int index = py2sqMapIndexOf(map, pyObj);
  PyObject **pyData = map->pyObj;
  OOP      *sqData = vm->firstIndexableField(map->sqObj);
  OOP       nilOop = vm->nilObject(); /* we'll need that a couple of times */

  /* SUBTLETY WARNING:
     The usage of sqData[] only works because we're only changing
     pointers inside it, meaning that we do not need to guard root
     stores. If that were different we'd have to explicitly use
     storePointerofObjectwithValue to guard those stores.
  */

  if(index == -1) return 0;

  if(pyData[index] != pyObj) {
    /* PyObj wasn't in the map to begin with */
    pyErr = "py2sqUnmap: PyObject not present in map";
    return 0;
  }

  map->size--;
  pyData[index] = NULL;
  sqData[index] = nilOop;

  /* fix collisions starting at index */
  oldIndex = (index+1) % map->max;
  while(pyData[oldIndex] != NULL) {
    newIndex = py2sqMapIndexOf(map, pyData[oldIndex]);
    if(newIndex != oldIndex) {
      pyData[newIndex] = pyData[oldIndex];
      pyData[oldIndex] = NULL;
      sqData[newIndex] = sqData[oldIndex];
      sqData[oldIndex] = nilOop;
    }
    oldIndex = (oldIndex+1) % map->max;
  }
  Py_DECREF(pyObj);
  return 1;
}
