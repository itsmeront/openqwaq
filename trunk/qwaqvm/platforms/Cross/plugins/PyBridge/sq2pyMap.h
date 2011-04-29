/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-20011, Teleplace, Inc., All Rights Reserved
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

#ifndef SQ2PYMAP_H
#define SQ2PYMAP_H

/* This is essentially the same as Py2SqMap just indexed by Squeak objects */

typedef struct Sq2PyMap {
  int size;
  int max;
  OOP       sqObj;
  PyObject **pyObj;
} Sq2PyMap;

extern Sq2PyMap *sq2pyMap;

/* sq2pyMapNew: Create a new sq2py map.
   Returns the new map or NULL if not successful.
   Arguments:
     nItems: The initial size of the map.
*/
Sq2PyMap *sq2pyMapNew(int nItems);

/* sq2pyMapFree: Free the given sq2py map.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The map to be freed.
*/
int sq2pyMapFree(Sq2PyMap *map);

/* sq2pyMapIndexOf: Find the slot index for a PyObject in the given map.
   Returns the index for the slot or -1 on error.
   Arguments:
     map: The Sq2PyMap to use.
     sqObj: The Squeak OOP to find.
*/
int sq2pyMapIndexOf(Sq2PyMap *map, OOP sqObj);

/* sq2pyMapFind: Find the PyObject for an SqObject.
   Returns the PyObject or NULL if not present.
   Arguments:
     map: The Sq2PyMap to search.
     sqObj: The Squeak object to find.
*/
PyObject *sq2pyMapFind(Sq2PyMap *map, OOP sqObj);

/* sq2pyMapGrow: Grow the map to contain more elements.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The map to grow.
*/
int sq2pyMapGrow(Sq2PyMap *map);

/* sq2pyMapAdd: Add an SqObject->PyObject association.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The Sq2PyMap to add to.
     sqObj: The Squeak OOP to use.
     pyObj: The PyObject reference to use.
*/
int sq2pyMapAdd(Sq2PyMap *map, OOP sqObj, PyObject *pyObj);

/* sq2pyUnmap: Undo the mapping of some Squeak OOP.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     map: The Sq2PyMap to remove the association from.
     sqObj: The OOP for the association to remove.
*/
int sq2pyUnmap(Sq2PyMap *map, OOP sqObj);

#endif /* SQ2PYMAP_H */
