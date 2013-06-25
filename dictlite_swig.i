// Copyright (c) 2013 Aubrey Barnard.  This is free software.  See
// LICENSE.txt for details.

// Creates a Python interface to Dictlite using Swig

%module dictlite_swig

// Headers to include
%header
%{
#include <Python.h>
#include "dictlite.h"
%}

// Auxilliary functions not part of the public interface
%inline %{

// Wrapper function for Python object comparison
static int
dictlite_swig_comparePyObjects(void * obj1, void * obj2)
{
  PyObject * pyObj1 = (PyObject *) obj1;
  PyObject * pyObj2 = (PyObject *) obj2;
  return PyObject_Compare(pyObj1, pyObj2);
}

static void
dictlite_swig_unrefContents(Dictlite * dict)
{
  DictliteItemIterator iterator = dictlite_itemIterator(dict);
  MappingItem * item;
  while ((item = dictlite_itemIterator_next(&iterator))) {
    Py_DECREF(item->key);
    Py_DECREF(item->value);
  }
}

%}


// Dictlite, as a class
typedef struct {
  MappingItem * head;
  MappingItem * end;
  size_t size;
  int (* compareKeys)(void * key1, void * key2);
} Dictlite;

%extend Dictlite {

  Dictlite() {
    return dictlite_new(dictlite_swig_comparePyObjects);
  }

  ~Dictlite() {
    dictlite_swig_unrefContents($self);
    dictlite_del($self);
  }

  size_t __len__() {
    return dictlite_size($self);
  }

  int __contains__(void * key) {
    return dictlite_contains($self, key);
  }

  void addFromDict(Dictlite * otherDict) {
    dictlite_addFromDict($self, otherDict);
  }
}
