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

  pyErr.c: Error handling support.

*/

#include "Python.h"

#include "sqVirtualMachine.h"
#include "sqConfig.h"
#include "sqPlatformSpecific.h"

#include "PyBridge.h"

char *pyErr = "Unknown Error";
char errBuf[1024];

/* pyErrPrint: Print a Python error message */
void pyErrPrint(char *defaultMsg) {
  PyObject *error, *value, *traceback, *string;

  pyErr = defaultMsg;
  PyErr_Fetch(&error, &value, &traceback);
  PyErr_NormalizeException(&error, &value, &traceback);
  if(error == NULL) return;
  string = PyObject_Str(value);
  if(string == NULL) {
    pyErr = "Unknown Python error (no description)";
  } else {
    sprintf(errBuf, "%s", PyString_AsString(string));
    pyErr = errBuf;
    Py_DECREF(string);
  }
}

/* pyGetLastError: Return the last error message */
char *pyGetLastError(void) {
  return pyErr;
}
