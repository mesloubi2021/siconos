// -*- C++ -*-
// Siconos is a program dedicated to modeling, simulation and control
// of non smooth dynamical systems.
//
// Copyright 2022 INRIA.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//

%module(package="siconos") fclib

%include start.i
%fragment("NumPy_Fragments");
%{
#ifdef __cplusplus
extern "C"
{
#endif

#include "fclib.h"

#ifdef __cplusplus
}
#endif
%}

 /* fclib_solution */
%typemap(in) (double *v) (SN_ARRAY_TYPE* array=NULL, int is_new_object = 0) {
  array = obj_to_array_allow_conversion($input, NPY_DOUBLE, &is_new_object);
  $1 = (double *) array_data(array);
 }

%apply (double *v) { (double *u) }
%apply (double *v) { (double *r) }
%apply (double *v) { (double *l) }

%define convert_fcsol_array(input, ptr) {
    void *argp1=0;
    int res1=0;
    int newmem=0;
    int i;
    if (!PySequence_Check(input)) {
      PyErr_SetString(PyExc_TypeError,"Expecting a sequence");
      SWIG_fail;
    }
    for (i =0; i <  PyObject_Length(input); i++) {
      PyObject *o = PySequence_GetItem(input,i);
      res1 = SWIG_ConvertPtrAndOwn(o, &argp1, $descriptor(fclib_solution *), 0 |  0 , &newmem);
      if (!SWIG_IsOK(res1)) {
        Py_XDECREF(o);
        PyErr_SetString(PyExc_ValueError,"Expecting a sequence of fclib_solution");
        SWIG_fail;
      }
      ptr[i] = *(struct fclib_solution*)(argp1);

//      if (ptr[i] == -1 && PyErr_Occurred())
//        SWIG_fail;
      Py_DECREF(o);
  }
  }
%enddef


%include fclib.h
