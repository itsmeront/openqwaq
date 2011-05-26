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
 
/**************************************************************************/
/*                        Object conversions                              */
/**************************************************************************/

/* sq2py: Create and return a PyObject from a Squeak oop.
   Returns a PyObject or NULL on error.
   Arguments:
     obj: The Squeak object reference to convert to Python.
*/
PyObject *sq2py(OOP obj);

/* py2sq: Create and return a Squeak oop from a PyObject.
   Returns the new OOP or NULL on failure.
   Arguments:
     obj: The PyObject reference to convert to Squeak.
*/
OOP py2sq(PyObject *obj);

/* py2sqGeneric: Create and return a Squeak oop from a PyObject.
   Does not do inline conversions of objects.
   Returns the new OOP or NULL on failure.
   Arguments:
     obj: The PyObject reference to convert to Squeak.
   
*/
OOP py2sqGeneric(PyObject *obj);

