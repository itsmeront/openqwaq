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

  py2sq.c: Squeak/Python interface implementation

*/

#include "Python.h"

#include "sqVirtualMachine.h"
#include "sqConfig.h"
/* #include "sqPlatformSpecific.h" */

#include "PyBridge.h"
#include "convert.h"

#include "sq2pyMap.h"
#include "py2sqMap.h"

/* max. callback depth */
#define MAX_CB_DEPTH 128

/* Callback information */
static int cbLevel = 0;         /* current callback depth */
static int sqCallbackSema = 0;  /* callback semaphore to signal */

/* rcvr, args, return values for callback */
static PyObject* rcvrMap[MAX_CB_DEPTH];
static PyObject* argsMap[MAX_CB_DEPTH];
static PyObject* retMap[MAX_CB_DEPTH];
static int idMap[MAX_CB_DEPTH];

/* call squeak from python */
PyObject *CallSqueak(PyObject *self, PyObject *args) {
  PyObject *pyResult;

  if(cbLevel+1 >= MAX_CB_DEPTH) {
    return NULL; /* e.g., error */
  }

  if(!sqCallbackSema) return 0; /* fail if no callback sema */

  /* this is cheesy but an easy workaround until I understand how this works */
  if(self == NULL) self = Py_None;

  /* make sure self, args don't get lost during the following */
  Py_INCREF(self);
  Py_INCREF(args);

  /* remember self, args */
  rcvrMap[cbLevel] = self;
  argsMap[cbLevel] = args;
  retMap[cbLevel] = NULL; /* meaning failure, not None */

  /* call back into the Squeak VM */
  vm->signalSemaphoreWithIndex(sqCallbackSema);
  vm->callbackEnter(&idMap[cbLevel++]);
  pyResult = retMap[--cbLevel];

  /* clean up and return the result */
  Py_DECREF(self);
  Py_DECREF(args);

  rcvrMap[cbLevel] = NULL;
  argsMap[cbLevel] = NULL;
  retMap[cbLevel] = NULL;

  return pyResult;
}

/* pyCallbackSetSemaphore: Associate a Squeak callback semaphore.
   Return non-zero if successful; zero otherwise.
   Arguments:
     semaIndex: The integer semaphore index to be signaled.
*/
int pyCallbackSetSemaphore(int semaIndex) {
  sqCallbackSema = semaIndex;
  return 1;
}

/* pyCallbackGetArgCount: Get the number of arguments for the callback.
   Returns the number of arguments or -1 if error.
   Does NOT include the receiver arg (index 0).
*/
int pyCallbackGetArgCount(void) {
  if(cbLevel <= 0 || cbLevel > MAX_CB_DEPTH) return -1;
  return PyObject_Length(argsMap[cbLevel-1]);
}

/* pyCallbackGetArg: Get the n-th argument for the callback.
   Returns OOP for the n-th argument or zero if error.
   Arguments:
     index: Index of the argument requested (0 - self, 1..n - args)
*/
OOP pyCallbackGetArg(int argIndex) {
  PyObject *pyArgs;
  OOP result = 0;
  int pySize;

  if(cbLevel <= 0 || cbLevel > MAX_CB_DEPTH) return 0;

  /* check for receiver */
  if(argIndex == 0) {
    result = py2sq(rcvrMap[cbLevel-1]);
    goto done;
  }
  /* pick up argument */
  pyArgs = argsMap[cbLevel-1];
  if(!pyArgs) {
    pyErr = "pyCallbackGetArg: pyArgs invalid (NULL)";
    goto done;
  }
  /* NOTE: The following range test is for 1-based indexing */
  pySize = PyObject_Length(pyArgs);
  if(argIndex > 0 && argIndex <= pySize) {
    result = py2sq(PyTuple_GetItem(pyArgs, argIndex-1));
  } else {
    /* Index out of bounds; fail */
    pyErr = "pyCallbackGetArg: Index out of bounds";
    result = 0;
  }
done:
  if(result)vm->pushRemappableOop(result);
  py2sqMapGrow(py2sqMap);
  sq2pyMapGrow(sq2pyMap);
  return result ? vm->popRemappableOop() : result;
}

/* pyCallbackSetResult: Set the result value to be returned from the call.
   Returns non-zero if successful.
   Arguments:
     sqReturnOop: The return value for the call.
*/
int pyCallbackSetResult(OOP sqReturnOop) {
  /* convert Squeak oop to Python */
  PyObject *pyResult;
  if(cbLevel <= 0 || cbLevel > MAX_CB_DEPTH) return 0;
  if(retMap[cbLevel-1]) return 0; /* fail; result already set */
  pyResult = sq2py(sqReturnOop);
  if(!pyResult) return 0; /* fail; something went wrong */
  retMap[cbLevel-1] = pyResult;
  sq2pyMapGrow(sq2pyMap);
  py2sqMapGrow(py2sqMap);
  return 1;
}

/* pyCallbackReturn: Return from a Squeak callback to the Python interpreter
   Returns non-zero if successful, zero otherwise.
   Arguments: None.
*/
int pyCallbackReturn(void) {
  if(cbLevel <= 0 || cbLevel > MAX_CB_DEPTH) return 0;
  return vm->callbackLeave(idMap[cbLevel-1]);
}

PyDoc_STRVAR(send_doc,
   "send(rcvr, selector, ...) \n\
\n\
Sends a message via the Python bridge.\n\
\n\
Arguments:\n\
  rcvr: The object receiving the message.\n\
  selector: The message name to send \n\
  args: An arbitrary list of arguments. \n\
\n\
Examples: \n\
  # Compute 3+4 in Squeak \n\
  Bridge.send(3,\"+\", 4) \n\
\n\
  # Get a reference to class Array \n\
  sqArrayClass = Bridge.send(Squeak, \"classNamed:\", \"Array\") \n\
\n\
  # Create a new Squeak array of size 4\ \n\
  sqArray = Bridge.send(sqArrayClass, \"new:\", 4)\ \n\
"
);

/* the module's function table */
PyMethodDef BridgeMethods[] = {
  {"send", CallSqueak, METH_VARARGS,  send_doc},
  {NULL, NULL, 0, NULL}       /* sentinel */
};
