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

  PythonPlugin.h: Squeak/Python interface definition

*/
#ifndef PYTHON_PLUGIN_H
#define PYTHON_PLUGIN_H

/* a little short hand for Squeak OOPs */
typedef int OOP;

/* a little short hand for Squeak's vm proxy */
extern struct VirtualMachine *interpreterProxy;
#define vm interpreterProxy

/**************************************************************************/
/*                        Module initializers                             */
/**************************************************************************/

/* pyStarted indicates whether we have successfully initialized the Python
   interpreter. This is needed since we may need to pass in a PYTHONHOME
   for the bridge and this needs to be set *before* we call Py_Initialize.
   However, unless we have called Py_Initialize, all the other functions
   provided here will break so this flag can be used to guard against
   invokations of these functions before the bridge is properly initialized.
*/
extern int pyStarted;

/* pyLaunch: Called when we wish to launch the Python interpreter.
   Arguments:
     pyHome: The value for PYTHONHOME.
   Returns:
   Non-zero if successful, zero otherwise. Also sets pyStarted if
   successful.
*/
int pyLaunch(char *pyHome);

/* pyUnload: Called to unload the Python interpreter explicitly.
   Arguments: None.
   Returns: Non-zero if successful, zero otherwise.
*/
int pyUnload(void);

/* pyInit: Called by Squeak when the PythonPlugin is loaded.
   Needs to initialize the Python interpreter accordingly.
   Return value: Non-zero if successful, zero otherwise.
*/
int pyInit(void);

/* pyExit: Called by Squeak when the PythonPlugin is unloaded.
   Needs to release the Python interpreter accordingly. 
   Return value: Non-zero if successful, zero otherwise.
*/
int pyExit(void);

/* cvtInit: Needs to be called exactly once before the conversions
   can be used. Preferredly this is done from pyInit.
   Return value: Non-zero if successful, zero otherwise.
*/
int cvtInit(void);

/**************************************************************************/
/*                        Python callout support                          */
/**************************************************************************/

/* pySetObjectClass: Set the PyObject class to use.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     objClass: The Squeak PyObject class to use.
*/
int pySetObjectClass(OOP objClass);

/* pyMapObjectTo: Set up a mapping between Python and Squeak objects.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     pyOop: Oop for the Python object to map.
     sqOop: Oop for the Squeak object to map.
*/
int pyMapObjectTo(OOP pyOop, OOP sqOop);

/* pyUnmapObject: Release a binding between a Squeak and a Python object.
   Returns non-zero if successful, zero otherwise.
   Arguments:
     sqOop: Oop for the Squeak object to unmap.
*/
int pyUnmapObject(OOP sqOop);

/* pyMappedObjects: Answer a copy of all currently mapped Squeak objects.
   Returns the OOP of the cloned array if successful, false otherwise.
   Arguments:
     None.
*/
OOP pyMappedObjects(void);

/* pyLoadModule: Load a Python module.
   Return the pyModule handle OOP or 0 on error. 
   Arguments:
     sqModuleOop: Squeak string with the name of the python module ('foo.py')
*/
int pyLoadModule(OOP sqModuleOop);

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
OOP pyCallMethod(OOP sqRcvrOop, OOP sqNameOop, OOP sqArgsOop);

/* pyBridgeVersion: Answer the version of the PyBridge */
#define PY_BRIDGE_VERSION "PyBridge 1.0 (alpha) from " __DATE__
static inline char *pyBridgeVersion(void) { return PY_BRIDGE_VERSION; }

/**************************************************************************/
/*                     Squeak callback support                            */
/**************************************************************************/

/* pyCallbackSetSemaphore: Associate a Squeak callback semaphore.
   Return non-zero if successful; zero otherwise.
   Arguments:
     semaIndex: The integer semaphore index to be signaled.
*/
int pyCallbackSetSemaphore(int semaIndex);

/* pyCallbackGetArg: Get the n-th argument for the callback.
   Returns OOP for the n-th argument or zero if error.
   Arguments:
     index: Index of the argument requested (0 - self, 1..n - args)
*/
OOP pyCallbackGetArg(int argIndex);

/* pyCallbackGetArgCount: Get the number of arguments for the callback.
   Returns the number of arguments or -1 if error.
   Arguments: None.
*/
int pyCallbackGetArgCount(void);

/* pyCallbackSetResult: Set the result value to be returned from the call.
   Returns non-zero if successful.
   Arguments:
     sqReturnOop: The return value for the call.
*/
int pyCallbackSetResult(OOP sqReturnOop);

/* pyCallbackReturn: Return from a Squeak callback to the Python interpreter
   Returns non-zero if successful, zero otherwise.
   Arguments: None.
*/
int pyCallbackReturn(void);

/**************************************************************************/
/*                     Error handling support                             */
/**************************************************************************/
extern char *pyErr;
extern char errBuf[1024];

/* pyGetLastError: Return the last error message */
char *pyGetLastError(void);
/* pyErrPrint: Print a Python error message */
void pyErrPrint(char *defaultMsg);

/**************************************************************************/
/*                     Debugging support                                  */
/**************************************************************************/

/* Use the LOG macro like here (note the double parens!!!):
   PyLOG((f,"Debug info: %d\n", 123))
*/
#define PyLOG(fprintfArg) { \
  FILE *f = fopen("PyBridge.log", "at"); \
  if(f) fprintf fprintfArg; \
  fclose(f); \
}

#endif /* PYTHON_PLUGIN_H */
