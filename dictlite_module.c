// Copyright (c) 2013 Aubrey Barnard.  This is free software.  See
// LICENSE.txt for details.

// Include Python first as its definitions may affect other headers
#include <Python.h>

#include "dictlite.h"


// A Python object wrapper for a Dictlite
struct dictlitemod_DictliteObject {
  PyObject_HEAD
  Dictlite * dl;
};
typedef struct dictlitemod_DictliteObject DictliteObject;

// Wrapper function for Python object comparison
static int
dictlitemod_comparePyObjects(void * obj1, void * obj2)
{
  PyObject * pyObj1 = (PyObject *) obj1;
  PyObject * pyObj2 = (PyObject *) obj2;
  return PyObject_Compare(pyObj1, pyObj2);
}

static PyObject *
dictlitemod_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
  // Allocate the memory for a new object
  DictliteObject * self = (DictliteObject *) type->tp_alloc(type, 0);
  if (self != NULL) {
    self->dl = dictlite_new(dictlitemod_comparePyObjects);
  }

  return (PyObject *) self;
}

static void
dictlitemod_unrefContents(Dictlite * dict)
{
  DictliteItemIterator iterator = dictlite_itemIterator(dict);
  MappingItem * item;
  while ((item = dictlite_itemIterator_next(&iterator))) {
    Py_DECREF(item->key);
    Py_DECREF(item->value);
  }
}

static void
dictlitemod_del(DictliteObject * self)
{
  dictlitemod_unrefContents(self->dl);
  dictlite_del(self->dl);
  self->ob_type->tp_free((PyObject *) self);
}

static Py_ssize_t
dictlitemod_size(DictliteObject * self)
{
  return dictlite_size(self->dl);
}

static int
dictlitemod_contains(DictliteObject * self, PyObject * key)
{
  return dictlite_contains(self->dl, key);
}

static void
dictlitemod_setKeyError(PyObject * key)
{
  // Key error (code based on dictobject.c)
  // Wrap the key in a tuple so its value remains intact and it is not unpacked as an argument list
  PyObject * wrappedKey = PyTuple_Pack(1, key);
  if (wrappedKey) {
    PyErr_SetObject(PyExc_KeyError, wrappedKey);
    Py_DECREF(wrappedKey);
  } else {
    // Couldn't wrap key
    PyErr_SetString(PyExc_KeyError, "Key error and could not wrap key in exception.");
  }
}

static PyObject *
dictlitemod_getValue(DictliteObject * self, PyObject * key)
{
  PyObject * value = (PyObject *) dictlite_getValue(self->dl, key);
  if (value == NULL) {
    dictlitemod_setKeyError(key);
    return NULL;
  } else {
    // Return the value
    // Must return a new reference
    Py_INCREF(value);
    return value;
  }
}

static int
dictlitemod_setValue(DictliteObject * self, PyObject * key, PyObject * value)
{
  // Based on dictobject.c the value can be NULL which means delete the mapping
  // Return -1 on failure
  if (value == NULL) {
    MappingItem * item = dictlite_delItem(self->dl, key);
    if (item == NULL) {
      // No such key
      dictlitemod_setKeyError(key);
      // Failed delete
      return -1;
    } else {
      // Key and value were found, release them from this dict
      Py_DECREF((PyObject *) item->key);
      Py_DECREF((PyObject *) item->value);
      free(item);
      // Successful delete
      return 0;
    }
  } else {
    void * oldValue = dictlite_setValue(self->dl, key, value);
    if (oldValue == NULL) {
      // New mapping was added (NULL -> no previous mapping)
      Py_INCREF(key);
      Py_INCREF(value);
    } else {
      // No new mapping, just swapped the old and new values
      Py_DECREF((PyObject *) oldValue);
      Py_INCREF(value);
    }
    return 0;
  }
}

static Dictlite *
dictlitemod_newDictFromPyDict(PyObject * pyDict)
{
  int errorOccurred = 1;
  Dictlite * dict = NULL;
  PyObject * items = NULL;
  PyObject * itemsIterator = NULL;

  // Create objects for the iteration
  dict = dictlite_new(dictlitemod_comparePyObjects);
  if (dict == NULL)
    goto finally;
  items = PyMapping_Items(pyDict);
  if (items == NULL)
    goto finally;
  itemsIterator = PyObject_GetIter(items);
  if (itemsIterator == NULL)
    goto finally;

  // Add the items from the other dict
  PyObject * item;
  PyObject * key;
  PyObject * value;
  while ((item = PyIter_Next(itemsIterator))) {
    if (PyTuple_Check(item) && PyArg_ParseTuple(item, "OO", &key, &value)) {
      Py_INCREF(key);
      Py_INCREF(value);
      dictlite_setValue(dict, key, value);
    } else {
      PyErr_Format(PyExc_TypeError, "Expected a (key, value) pair not a '%s'.", item->ob_type->tp_name);
      Py_DECREF(item);
      break;
    }
    Py_DECREF(item);
  }
  if (!PyErr_Occurred())
    errorOccurred = 0;

 finally:
  Py_XDECREF(itemsIterator);
  Py_XDECREF(items);
  if (errorOccurred) {
    // Clean up the constructed dict
    if (dict) {
      dictlitemod_unrefContents(dict);
      dictlite_del(dict);
    }
    return NULL;
  } else {
    return dict;
  }
}

static PyObject *
dictlitemod_addFromDict(DictliteObject * self, PyObject * args)
{
  int errorOccurred = 1;
  PyObject * otherDictObj = NULL;
  Dictlite * otherDict = NULL;

  // Parse and check arguments
  if (!PyArg_ParseTuple(args, "O:addFromDict", &otherDictObj))
    goto finally;
  Py_INCREF(otherDictObj);  // Own otherDictObj
  if (!PyMapping_Check(otherDictObj)) {
    PyErr_Format(PyExc_TypeError, "Expected a mapping not a '%s'.", otherDictObj->ob_type->tp_name);
    goto finally;
  }

  // Create a dict from the other dict
  otherDict = dictlitemod_newDictFromPyDict(otherDictObj);
  if (otherDict == NULL)
    goto finally;

  // Add the mappings from the created other dict to self
  dictlite_addFromDict(self->dl, otherDict);
  errorOccurred = 0;

 finally:
  // Clean up created dict
  if (otherDict) {
    dictlitemod_unrefContents(otherDict);
    dictlite_del(otherDict);
  }
  // Clean up Python objects
  Py_XDECREF(otherDictObj);
  // Return error or None
  if (errorOccurred)
    return NULL;
  Py_RETURN_NONE;
}

// Python sequence methods for Dictlite
static PySequenceMethods dictlitemod_as_sequence = {
  (lenfunc) dictlitemod_size,  // sq_length
  0,  // sq_concat
  0,  // sq_repeat
  0,  // sq_item
  0,  // sq_slice
  0,  // sq_ass_item
  0,  // sq_ass_slice
  (objobjproc) dictlitemod_contains,  // sq_contains
};

static PyMappingMethods dictlitemod_as_mapping = {
  (lenfunc) dictlitemod_size,  // mp_length
  (binaryfunc) dictlitemod_getValue,  // mp_subscript
  (objobjargproc) dictlitemod_setValue,  // mp_ass_subscript
};

// Methods of the Dictlite type
static PyMethodDef dictlitemod_methods[] = {
  {"addFromDict", (PyCFunction) dictlitemod_addFromDict, METH_VARARGS, "Adds the mappings contained in the given dict to this dict."},
  {NULL, NULL, 0, NULL}  // Sentinel
};

// The Python Dictlite type
static PyTypeObject dictlitemod_type = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,  // ob_size
  "dictlite.Dictlite",  // tp_name
  sizeof(DictliteObject),  // tp_basicsize
  0,  // tp_itemsize
  (destructor) dictlitemod_del,  // tp_dealloc
  0,  // tp_print
  0,  // tp_getattr
  0,  // tp_setattr
  0,  // tp_compare
  0,  // tp_repr
  0,  // tp_as_number
  &dictlitemod_as_sequence,  // tp_as_sequence
  &dictlitemod_as_mapping,  // tp_as_mapping
  0,  // tp_hash
  0,  // tp_call
  0,  // tp_str
  0,  // tp_getattro
  0,  // tp_setattro
  0,  // tp_as_buffer
  Py_TPFLAGS_DEFAULT,  // tp_flags
  "Lightweight dictionary object",  // tp_doc
  0,  // tp_traverse
  0,  // tp_clear
  0,  // tp_richcompare
  0,  // tp_weaklistoffset
  0,  // tp_iter
  0,  // tp_iternext
  dictlitemod_methods,  // tp_methods
  0,  // tp_members
  0,  // tp_getset
  0,  // tp_base
  0,  // tp_dict
  0,  // tp_descr_get
  0,  // tp_descr_set
  0,  // tp_dictoffset
  0,  // tp_init
  0,  // tp_alloc
  dictlitemod_new,  // tp_new
};

// Module methods
static PyMethodDef dictlitemod_module_methods[] = {
  {NULL}
};

// Module initialization
PyMODINIT_FUNC
initdictlite()
{
  if (PyType_Ready(&dictlitemod_type) < 0)
    return;

  PyObject * module = Py_InitModule3("dictlite", dictlitemod_module_methods, "Lightweight dictionary object module.");
  if (module == NULL)
    return;

  Py_INCREF(&dictlitemod_type);
  PyModule_AddObject(module, "Dictlite", (PyObject *) &dictlitemod_type);
}
