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

/*

  sq2py.c: Squeak/Python interface implementation

*/

#include "Python.h"

#include "sqVirtualMachine.h"
#include "sqConfig.h"
#include "sqPlatformSpecific.h"

#include "PyBridge.h"
#include "convert.h"
#include "py2sqMap.h"
#include "sq2pyMap.h"

OOP ClassPyObject = 0;

/* pySetObjectClass: Set the PyObject class to use.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     objClass: The Squeak PyObject class to use.
*/
int pySetObjectClass(OOP sqPyClass) {
  if(vm->isIntegerObject(sqPyClass)) return 0;
  if(vm->nilObject() != sqPyClass) {
    if(ClassPyObject == 0) vm->addGCRoot(&ClassPyObject);
    ClassPyObject = sqPyClass;
  } else {
    if(ClassPyObject != 0) vm->removeGCRoot(&ClassPyObject);
    ClassPyObject = 0;
  }
  return 1;
}

/* pyLoadModule: Load a Python module.
   Return the pyModule handle OOP or 0 on error. 
   Arguments:
     sqModuleOop: Handle to previuously loaded pyModule.
*/
OOP pyLoadModule(OOP sqModuleOop) {
  PyObject *pyModule = NULL, *pyModuleName = NULL;
  OOP result = 0;

  if(!pyStarted) {
    pyErr = "pyLoadModule: Bridge uninitialized";
    return 0;
  }

  /* Convert to Module name to PyString */
  if(vm->fetchClassOf(sqModuleOop) != vm->classString()) {
    pyErr = "pyLoadModule: Module name is not (Byte)String";
    goto failure;
  }
  pyModuleName = sq2py(sqModuleOop);
  if(!pyModuleName) {
    pyErr = "pyLoadModule: sq2py(sqModuleName) failed";
    goto failure;
  }
  /* Load module */
  pyModule = PyImport_Import(pyModuleName);
  if(!pyModule) {
    pyErrPrint("pyLoadModule: PyImport_Import() failed");
    goto failure;
  }
  result = py2sq(pyModule);
failure:
  Py_XDECREF(pyModuleName);
  Py_XDECREF(pyModule);
  if(result) vm->pushRemappableOop(result);
  py2sqMapGrow(py2sqMap);
  sq2pyMapGrow(sq2pyMap);
  return result ? vm->popRemappableOop() : result;
}

/* pyCallMethod: Call a method on an Object.
   Returns the Squeak OOP of the result or NULL on error.
   Arguments:
     sqRcvrOop: The PyObject to send the message to.
     sqNameOop: The name of the message to send.
     sqArgsOop: The arguments to the call.
   This function is an optimization for pyCallFunction()
   in such that it combines the pyGetAttr() and pyCallFunction()
   and avoids extra overhead on the garbage collector.
*/
OOP pyCallMethod(OOP sqRcvrOop, OOP sqNameOop, OOP sqArgsOop) {
  PyObject *pySelf = sq2py(sqRcvrOop);
  PyObject *pyName = sq2py(sqNameOop);
  PyObject *pyFunction = NULL, *pyTuple = NULL, *pyResult = NULL;
  OOP sqResult = 0;
  int i, sz;

  if(!pyStarted) {
    pyErr = "pyLoadModule: Bridge uninitialized";
    return 0;
  }

  if(pySelf == NULL || pyName == NULL) {
    pyErrPrint("pyCallMethod: Invalid receiver or method name");
    goto failure;
  }
  PyString_InternInPlace(&pyName);
  pyFunction = PyObject_GetAttr(pySelf, pyName);
  if(pyFunction == NULL || !PyCallable_Check(pyFunction)) {
    pyErrPrint("pyCallMethod: Attribute not callable");
    goto failure;
  }

  /* Convert Squeak args to Python */
  if(!vm->isArray(sqArgsOop)) {
    pyErr = "pyCallMethod: arguments are not an array";
    goto failure;
  }
  sz = vm->slotSizeOf(sqArgsOop); /* how many? */
  pyTuple = PyTuple_New(sz);
  if(pyTuple == NULL) {
    pyErrPrint("pyCallMethod: PyTuple_New failed");
    goto failure;
  }

  for(i=0; i<sz; i++) {
    PyObject *pyArg;
    vm->pushRemappableOop(sqArgsOop);
    pyArg = sq2py(vm->fetchPointerofObject(i, sqArgsOop));
    sqArgsOop = vm->popRemappableOop();
    if(!pyArg) {
      pyErr = "pyCallMethod: argument conversion (sq2py) failed";
      goto failure;
    }
    PyTuple_SetItem(pyTuple, i, pyArg);
    /* do NOT DECREF(pyArg) since PyTuple_SetItem steals references */
  }
  /* Call function */
  pyResult = PyObject_CallObject(pyFunction, pyTuple);
  if(pyResult == NULL) {
    pyErrPrint("pyCallMethod: PyObject_CallObject() failed");
    goto failure;
  }
  /* Convert result back from Python to Squeak and clean up */
  sqResult = py2sq(pyResult);
  if(!sqResult) {
    pyErr = "pyCallMethod: result conversion (py2sq) failed";
    goto failure;
  }
failure:
  Py_XDECREF(pySelf);
  Py_XDECREF(pyName);
  Py_XDECREF(pyFunction);
  Py_XDECREF(pyTuple);
  Py_XDECREF(pyResult);
  if(sqResult) vm->pushRemappableOop(sqResult);
  py2sqMapGrow(py2sqMap);
  sq2pyMapGrow(sq2pyMap);
  if (sqResult) sqResult = vm->popRemappableOop();
  return sqResult;
}

/* pyMapObjectTo: Set up a mapping between Python and Squeak objects.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     pyOop: Oop for the Python object to map.
     sqOop: Oop for the Squeak object to map.
*/
int pyMapObjectTo(OOP pyOop, OOP sqOop) {
  PyObject *pyObj = sq2py(pyOop);
  PyObject *pyOld;
  OOP sqOld;

  if(!pyObj) return 0;
  /* Clean up the old mapping for pyObj */
  sqOld = py2sqMapFind(py2sqMap, pyObj);
  if(sqOld) {
    sq2pyUnmap(sq2pyMap, sqOld);
    py2sqUnmap(py2sqMap, pyObj);
  }
  /* Clean up the old mapping for sqObj (if any) */
  pyOld = sq2pyMapFind(sq2pyMap, sqOop);
  if(pyOld) {
    py2sqUnmap(py2sqMap, pyOld);
    sq2pyUnmap(sq2pyMap, sqOop);
  }
  py2sqMapAdd(py2sqMap, pyObj, sqOop);
  sq2pyMapAdd(sq2pyMap, sqOop, pyObj);
  Py_DECREF(pyObj);
  sq2pyMapGrow(sq2pyMap);
  py2sqMapGrow(py2sqMap);
  return 1;
}

/* pyUnmapObject: Release a binding between a Squeak and a Python object.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     sqOop: Oop for the Squeak object to unmap.
*/
int pyUnmapObject(OOP sqObj) {
  PyObject *pyObj = sq2pyMapFind(sq2pyMap, sqObj);
  if(pyObj) {
    sq2pyUnmap(sq2pyMap, sqObj);
    py2sqUnmap(py2sqMap, pyObj);
  }
  return 1;
}

/* pyMappedObjects: Answer a copy of all currently mapped Squeak objects.
   Returns the OOP of the cloned array if successful, false otherwise.
   Arguments:
     None.
*/
OOP pyMappedObjects(void) {
  return interpreterProxy->clone(py2sqMap->sqObj);
}

