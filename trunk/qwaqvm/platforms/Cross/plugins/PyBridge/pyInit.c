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

   pyInit.c: Initialization/Shutdown functions.

*/

#include "Python.h"

#include "sqVirtualMachine.h"
#include "sqConfig.h"
#include "sqPlatformSpecific.h"

#include "PyBridge.h"
#include "convert.h"

extern PyMethodDef BridgeMethods[];

/* pyStarted indicates whether we have successfully initialized the Python
   interpreter. This is needed since we may need to pass in a PYTHONHOME
   for the bridge and this needs to be set *before* we call Py_Initialize.
   However, unless we have called Py_Initialize, all the other functions
   provided here will break so this flag can be used to guard against
   invokations of these functions before the bridge is properly initialized.
*/
int pyStarted = 0;

/* pyInit: Called by Squeak when the PythonPlugin is loaded.
   Needs to initialize the Python interpreter accordingly.
   Return value: Non-zero if successful, zero otherwise.
*/
int pyInit(void) {
  /* this no longer does anything since we require explicit pyLaunch */
  return 1;
}

/* pyExit: Called by Squeak when the PythonPlugin is unloaded.
   Needs to release the Python interpreter accordingly. 
   Return value: Non-zero if successful, zero otherwise.
*/
int pyExit(void) {
  return 0; /* disallow unloading */
}

/* pyUnload: Explicitly unload the Python interpreter. */
int pyUnload(void) {
  pyStarted = 0;
  Py_Finalize();
  return 1;
}

/* pyLaunch: Called when we wish to launch the Python interpreter.
   Arguments:
     pyHome: The value for PYTHONHOME.
   Returns:
   Non-zero if successful, zero otherwise. Also sets pyStarted if
   successful.
*/
int pyLaunch(char *pyHome) {
  PyObject *module;

  if(pyStarted) return 1; /* already started */

  if(pyHome) {
    /* set home before proceeding */
    Py_SetPythonHome(pyHome);
  }

  /* fixme: needs to be PyMac_Init on Macs; what is the proper #define? */
  Py_Initialize();
  module = Py_InitModule("Squeak", BridgeMethods);
  PyModule_AddObject(module, "Smalltalk", _PyObject_New(&PyBaseObject_Type));

  module = PyImport_AddModule("<PyBridge - Special Objects>");
  if(module == NULL) {
    Py_Finalize();
    return 0;
  }
  PyModule_AddObject(module, "PyObject", (PyObject*)&PyBaseObject_Type);

  PyModule_AddObject(module, "PyClass", (PyObject*)&PyClass_Type);
  PyModule_AddObject(module, "PyInstance", (PyObject*)&PyInstance_Type);

  PyModule_AddObject(module, "PyType", (PyObject*)&PyType_Type);
  PyModule_AddObject(module, "PyModule", (PyObject*)&PyModule_Type);
  PyModule_AddObject(module, "PyFunction", (PyObject*)&PyFunction_Type);
  PyModule_AddObject(module, "PyTuple", (PyObject*)&PyTuple_Type);
  PyModule_AddObject(module, "PyMethod", (PyObject*)&PyMethod_Type);
  PyModule_AddObject(module, "PyList", (PyObject*)&PyList_Type);
  PyModule_AddObject(module, "PyDict", (PyObject*)&PyDict_Type);

  PyModule_AddObject(module, "PyInt", (PyObject*)&PyInt_Type);
  PyModule_AddObject(module, "PyLong", (PyObject*)&PyLong_Type);
  PyModule_AddObject(module, "PyFloat", (PyObject*)&PyFloat_Type);
  PyModule_AddObject(module, "PyString", (PyObject*)&PyString_Type);

  PyModule_AddObject(module, "EmptyTuple", PyTuple_New(0));

  cvtInit();

  pyStarted = 1;

  return 1;
}
