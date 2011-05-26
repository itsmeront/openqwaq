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

  convert.c: Convert between Python and Squeak objects

*/
#include "Python.h"

#include "sqVirtualMachine.h"
#include "sqConfig.h"
/* #include "sqPlatformSpecific.h" */

#include "PyBridge.h"
#include "convert.h"
#include "py2sqMap.h"
#include "sq2pyMap.h"

/* Large integer conversion buffer */
static unsigned int longBufMax = 0;
static unsigned char *longBuf = NULL;
Sq2PyMap *sq2pyMap = NULL;
Py2SqMap *py2sqMap = NULL;

extern OOP ClassPyObject;

/* cvtInit: Initialize conversion methods. */
int cvtInit(void) {
  sq2pyMap = sq2pyMapNew(123);
  py2sqMap = py2sqMapNew(123);
  return (sq2pyMap != NULL) && (py2sqMap != NULL);
}

/* sq2py: Create and return a PyObject from a Squeak oop.
   Returns a PyObject or NULL on error.
   Arguments:
     obj: The Squeak object reference to convert to Python.
*/
PyObject *sq2py(OOP obj) {
  OOP objClass = vm->fetchClassOf(obj);

  if(!pyStarted) {
    pyErr = "sq2py: Bridge not initialized";
    return NULL;
  }
  /* nil, true, false */
  if(obj == vm->nilObject()) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  if(obj == vm->trueObject()) {
    Py_INCREF(Py_True);
    return Py_True;
  }
  if(obj == vm->falseObject()) {
    Py_INCREF(Py_False);
    return Py_False;
  }

  /* Small integer type (31bit) */
  if(objClass == vm->classSmallInteger()) {
    return PyInt_FromLong(vm->signed32BitValueOf(obj));
  }

  /* Large integer types (32 bits or more) */
  if(objClass == vm->classLargePositiveInteger() ||
     objClass == vm->classLargeNegativeInteger()) {
    int i, nBytes = vm->byteSizeOf(obj);
    unsigned char *bytes = vm->firstIndexableField(obj);

    /* Cover 32 bit signed ints */
    if(nBytes <= 4 && bytes[3] <= 127) {
      return PyLong_FromLong(vm->signed32BitValueOf(obj));
    }

    /* Cover 64 bit signed ints */
    if(nBytes <= 8 && bytes[7] <= 127) {
      return PyLong_FromLongLong(vm->signed64BitValueOf(obj));
    }

    /* Cover LargePositiveInteger */
    if(objClass == vm->classLargePositiveInteger()) {
      return _PyLong_FromByteArray(bytes, nBytes, 1, 0);
    }
    /* The remaining case dealing with LargeNegativeInteger outside of
       the 64bit range needs to compute the 2s complement of the magnitude
       (even though Python doesn't seem to use that, sigh...) */
    if(nBytes+1 >= longBufMax) { 
      longBufMax = nBytes+1; /* one extra for sign */
      if(longBuf) free(longBuf);
      longBuf = malloc(longBufMax);
    }
    memcpy(longBuf, bytes, nBytes);
    for(i=0; i < nBytes; i++) longBuf[i] ^= 255;
    for(i=0; i < nBytes; i++) if(++longBuf[i]) break;
    if(longBuf[nBytes-1] < 128) longBuf[nBytes++] = 255; /* sign bit */
    return _PyLong_FromByteArray(longBuf, nBytes, 1, 1);
  }

  /* 64bit floating point value */
  if(objClass == vm->classFloat()) {
    return PyFloat_FromDouble(vm->floatValueOf(obj));
  }

  /* byte strings */
  if(objClass == vm->classString()) {
    char *strPtr = vm->firstIndexableField(obj); 
    int sz = vm->byteSizeOf(obj);
    return PyString_FromStringAndSize(strPtr, sz);
  }

  /* array conversions */
  if(objClass == vm->classArray()) { 
    int i, sz = vm->slotSizeOf(obj);
    PyObject *tuple, *item;
    tuple = PyTuple_New(sz);
    for(i=0; i<sz; i++) {
      item = sq2py(vm->fetchPointerofObject(i, obj));
      if(item == NULL) return NULL;
      PyTuple_SetItem(tuple, i, item);
    }
    return tuple;
  }

  /* Generic object conversions */
  {
    PyObject *pyObj;
    /* First, look for a generic translation */
    pyObj = sq2pyMapFind(sq2pyMap, obj);
    if(pyObj != NULL) {
      Py_INCREF(pyObj);
      return pyObj;
    }
    /* TBD: look for a class/type translation */
  }

  /* For everything else just FAIL for now */
  pyErr = "sq2py: Unknown Squeak type";
  return NULL;
}

/* py2sq: Create and return a Squeak oop from a PyObject.
   Returns the new OOP or NULL on failure.
   Arguments:
     obj: The PyObject reference to convert to Squeak.
*/
OOP py2sq(PyObject *obj) {

  if(obj == NULL) {
    pyErr = "py2sq: NULL PyObject reference encountered";
    return 0;
  }
  /* nil, true, false */
  if(obj == Py_None) {
    return vm->nilObject();
  }
  if(obj == Py_True) {
    return vm->trueObject();
  }
  if(obj == Py_False) {
    return vm->falseObject();
  }

  /* 32bit signed integer value */
  if(PyObject_TypeCheck(obj, &PyInt_Type)) {
    return vm->signed32BitIntegerFor(PyInt_AsLong(obj));
  }

  /* BigNums */
  if(PyObject_TypeCheck(obj, &PyLong_Type)) {
    int nBits = _PyLong_NumBits(obj);
    int i, nBytes = (nBits + 7) / 8;
    OOP sqInt, sqIntClass;

    /* Cover PyLongs <= 32 bit */
    if(nBits < 32) {
      int value = PyLong_AsLong(obj);
      return vm->signed32BitIntegerFor(value);
    }
#if 0
    /* At this point (VM proxy 1.x), Squeak's signed64BitIntegerFor()
       is HORRIBLY broken. Once a fixed version exists the above
       should be defined as if VM_PROXY_MINOR > xxx to enable the 
       fast conversion below */

    /* Cover PyLongs <= 64 bit */
    if(nBits < 64) {
      long long veryLong = PyLong_AsLongLong(obj);
      return vm->signed64BitIntegerFor(veryLong);
    }
#endif

    /* Cover large positive integers */
    if(_PyLong_Sign(obj) >= 0) {
      sqIntClass = vm->classLargePositiveInteger();
      sqInt = vm->instantiateClassindexableSize(sqIntClass, nBytes);
      _PyLong_AsByteArray((PyLongObject*)obj, vm->firstIndexableField(sqInt), 
			  nBytes, 1, 0);
      return sqInt;
    }

    /* Cover the remaining case of large negative integers.
       Unfortunately, Python only gives us an interface using the 2s
       complement so we have to recompute the magnitude from it. Sigh. */
    nBytes++; /* one extra in case we need the sign bit */
    if(nBytes >= longBufMax) { 
      longBufMax = nBytes;
      if(longBuf) free(longBuf);
      longBuf = malloc(longBufMax);
    }
    _PyLong_AsByteArray((PyLongObject*)obj, longBuf, nBytes, 1, 1);
    for(i=0; i < nBytes; i++) longBuf[i] ^= 255;
    for(i=0; i < nBytes; i++) if(++longBuf[i]) break;
    while(longBuf[nBytes-1] == 0) nBytes--;
    sqIntClass = vm->classLargeNegativeInteger();
    sqInt = vm->instantiateClassindexableSize(sqIntClass, nBytes);
    memcpy(vm->firstIndexableField(sqInt), longBuf, nBytes);
    return sqInt;
  }

  /* 64bit double float value */
  if(PyObject_TypeCheck(obj, &PyFloat_Type)) {
    return vm->floatObjectOf(PyFloat_AsDouble(obj));
  }

  /* string -- only deals with byte strings here */
  if(PyObject_TypeCheck(obj, &PyString_Type)) {
    int sz = PyString_Size(obj);
    char *src = PyString_AsString(obj);
    OOP strOop = vm->instantiateClassindexableSize(vm->classString(), sz);
    char *dst = vm->firstIndexableField(strOop);
    memcpy(dst, src, sz);
    return strOop;
  }

  /* tuples -- convert those to arrays */
  if(PyObject_TypeCheck(obj, &PyTuple_Type)) {
    int i, sz;
    OOP arrayOop, itemOop;
    sz = PyObject_Length(obj);
    arrayOop = vm->instantiateClassindexableSize(vm->classArray(), sz);
    for(i = 0; i < sz; i++) {
      vm->pushRemappableOop(arrayOop);
      itemOop = py2sq(PyTuple_GetItem(obj, i));
      arrayOop = vm->popRemappableOop();
      if(itemOop == 0) return 0;
      vm->storePointerofObjectwithValue(i, arrayOop, itemOop);
    }
    return arrayOop;
  }

  return py2sqGeneric(obj);
}

OOP py2sqGeneric(PyObject *obj) {
  /* Generic Python to Squeak conversion */

  if(!ClassPyObject) {
    pyErr = "py2sq: Unknown Python type (PyObject not initialized?)";
    return 0;
  }

  /* First check if there is a literal translation for the object */
  {
    /* First, look for a literal translation */
    OOP sqObj = py2sqMapFind(py2sqMap, obj);
    if(sqObj != 0) return sqObj;
  }

  /* Second, instantiate a new instance of the matching type */
  {
    PyTypeObject *pyType = obj->ob_type;
    OOP sqClass = 0;
    OOP sqObj;

    /* Find the class to instantiate */
    while(sqClass == 0) {
      if(pyType == NULL) {
	sqClass = ClassPyObject;
      } else {
	sqClass = py2sqMapFind(py2sqMap, (PyObject*)pyType);
	if(sqClass) {
	  /* Ensure that sqClass is a subclass of ClassPyObject.
	     The test below is tricky since we need to make sure
	     that sqClass is indeed a class. The way to do that
	     is by testing the meta-class hierarchy. */
 	  if(!vm->includesBehaviorThatOf(vm->fetchClassOf(sqClass),
					 vm->fetchClassOf(ClassPyObject))) {
	    /* Not a valid metaclass; dump it */
	    sqClass = 0;
	  }
	}
	/* Lookup the base class for the type */
	pyType = pyType->tp_base;
      }
    }
    sqObj = vm->instantiateClassindexableSize(sqClass, 0);
    vm->pushRemappableOop(sqObj);
    py2sqMapAdd(py2sqMap, obj, sqObj);
    sqObj = vm->popRemappableOop();
    sq2pyMapAdd(sq2pyMap, sqObj, obj);
    return sqObj;
  }
}
