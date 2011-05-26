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

#ifndef PY2SQMAP_H
#define PY2SQMAP_H

/* This is essentially the same as Sq2PyMap just indexed by Python objects */
typedef struct Py2SqMap {
  int size;
  int max;
  PyObject **pyObj;
  OOP       sqObj;
} Py2SqMap;

extern Py2SqMap *py2sqMap;

/* py2sqMapNew: Create a new py2sq map.
   Returns the new map or NULL if not successful.
   Arguments:
     nItems: The initial size of the map.
*/
Py2SqMap *py2sqMapNew(int nItems);

/* py2sqMapFree: Free the given py2sq map.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The map to be freed.
*/
int py2sqMapFree(Py2SqMap *map);

/* py2sqMapIndexOf: Find the slot index for a PyObject in the given map.
   Returns the index for the slot or -1 on error.
   Arguments:
     map: The Py2SqMap to use.
     pyObj: The PyObject to find.
*/
int py2sqMapIndexOf(Py2SqMap *map, PyObject *pyObj);

/* py2sqMapFind: Find the Sqeak OOP for a PyObject.
   Returns the OOP or zero if not present.
   Arguments:
     map: The Py2SqMap to search.
     pyObj: The Squeak object to find.
*/
OOP py2sqMapFind(Py2SqMap *map, PyObject *pyObj);

/* py2sqMapGrow: Grow the map to contain more elements.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The map to grow.
*/
int py2sqMapGrow(Py2SqMap *map);

/* py2sqMapAdd: Add a PyObject->Squeak OOP association.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The Sq2PyMap to add to.
     pyObj: The PyObject reference to use.
     sqObj: The Squeak OOP to use.
*/
int py2sqMapAdd(Py2SqMap *map, PyObject *pyObj, OOP sqObj);

/* py2sqUnmap: Undo the mapping of some PyObject.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The Py2SqMap to remove the association from.
     pyObj: The PyObject for the association to remove.
*/
int py2sqUnmap(Py2SqMap *map, PyObject *pyObj);

#endif /* PY2SQMAP_H */
