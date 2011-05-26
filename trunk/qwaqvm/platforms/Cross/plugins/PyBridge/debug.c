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

  debug.c: (Hopefully) temporary debugging code

  THIS FILE IS NOT BEING BUILT BY DEFAULT.
*/
#include "Python.h"

#include "sqVirtualMachine.h"
#include "sqConfig.h"
/* #include "sqPlatformSpecific.h" */

#include "PyBridge.h"
#include "py2sqMap.h"
#include "sq2pyMap.h"

static void printMaps(void) {
  int i;
  OOP sqObj;
  PyObject *pyObj;
  PyLOG((f,"py2sqMap (size: %d, max: %d)\n", py2sqMap->size, py2sqMap->max));
  for(i=0; i<py2sqMap->max; i++) {
    pyObj = py2sqMap->pyObj[i];
    sqObj = vm->fetchPointerofObject(i, py2sqMap->sqObj);
    PyLOG((f,"\t%d\tpyObj: %x\t sqObj: %x\n", i, pyObj, sqObj));
  }
  PyLOG((f,"sq2pyMap (size: %d, max: %d)\n", sq2pyMap->size, sq2pyMap->max));
  for(i=0; i<sq2pyMap->max; i++) {
    pyObj = sq2pyMap->pyObj[i];
    sqObj = vm->fetchPointerofObject(i, sq2pyMap->sqObj);
    PyLOG((f,"\t%d:\tsqObj: %x\t pyObj: %x\n", i, sqObj, pyObj));
  }
}

int verifyMaps(char *function, int line) {
#if 0
  int i, index;
  OOP sqObj;
  PyObject *pyObj;
  FILE *f;

  if(function == NULL) function = "???";

  f = fopen("PyBridge.log", "at");
  fprintf(f, "%s -- %d\n", function, line);
  fclose(f);

  for(i=0; i<py2sqMap->max; i++) {
    pyObj = py2sqMap->pyObj[i];
    if(pyObj != NULL) {
      sqObj = vm->fetchPointerofObject(i, py2sqMap->sqObj);
      index = sq2pyMapIndexOf(sq2pyMap, sqObj);
      if(sq2pyMap->pyObj[index] != pyObj) {
	char s[256];
	sprintf(s, "verifyMap1: Map mismatch in %s (line %d)", function, line);
	f = fopen("PyBridge.log", "at");
	fprintf(f, "%s\n", s);
	fclose(f);
	printMaps();
	MessageBox(0, s, "PyBridge ERROR", MB_OK);
      }
    }
  }

  for(i=0; i<sq2pyMap->max; i++) {
    sqObj = vm->fetchPointerofObject(i, sq2pyMap->sqObj);
    if(sqObj != vm->nilObject()) {
      pyObj = sq2pyMap->pyObj[i];
      index = py2sqMapIndexOf(py2sqMap, pyObj);
      if(vm->fetchPointerofObject(index, py2sqMap->sqObj) != sqObj) {
	PyLOG((f, "verifyMap2: Map mismatch in %s (line %d)\n", 
	       function, line));
	PyLOG((f, "expected: %x, got: %x, nil: %x\n",
	       sqObj, vm->fetchPointerofObject(index, py2sqMap->sqObj),
	       vm->nilObject()));
	printMaps();
	MessageBox(0, "verifyMap2: Map mismatch", "PyBridge ERROR", MB_OK);
      }
    }
  }
#endif
  return 1;
}

