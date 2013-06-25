Dictlite
========


Dictlite is a simple, lightweight implementation of a dictionary in C.
It was conceived as a way to learn how to use the Python/C API and Swig
to connect C code to Python.  It exists on GitHub as an example of a
Python-C foreign function interface to be used for learning and
experimentation.  So play around with it!

The dictionary itself is implemented as a linked list of key-value pairs
suitable for small mappings, e.g. named parameters.

Due to its origins as a learning experience, I am afraid this code may
have some fairly naive and/or incomplete parts as well as bugs.
Evaluate it carefully.  I may at some point make some modifications and
improvements, but that is only likely if there is interest.  So fork it
or ask questions.

For a more mature but much more complicated example of a Python-C
interface see [my work on
libDAI](https://github.com/afbarnard/libdai/tree/pythonswig/swig).


License
-------

Dictlite is free, open source software.  You are permitted to copy and
use it however you wish.  It is released under the BSD 2-Clause License.
See the file `LICENSE.txt` in your distribution (or [on
GitHub](https://github.com/afbarnard/dictlite/blob/master/LICENSE.txt))
for details.


Contents
--------

* `dictlite.h`, `dictlite.c`: Dictlite dictionary data type
  implementation in C.

* `main.c`: Example program using dictlite.

* `dictlite_module.c`: Dictlite as a class for CPython.  This is the way
  to wrap C code for CPython by hand.  A good example of how to write C
  code extending CPython with regards to error handling, incrementing
  and decrementing reference counts, etc.

* `dictlite_swig.i`: Interface definition used by Swig to generate a
  dictlite wrapper for CPython.

* `setup.py`: Python build script for building the hand-wrapped code.

* `makefile`: Commands for building extension modules and deleting
  generated files.


Requirements
------------

* Standard Python (CPython) 2.x with development headers (Python.h)

* Swig 2.x

* GCC


Build
-----

```shell
$ make
```


Run
---

```shell
$ python
```

```python
# By-hand module
import dictlite as dl
d1 = dl.Dictlite()
d1['a'] = 1
d1[2] = 'b'
d1['c'] = 3
len(d1)  # 3
d1[2]  # 'b'
4 in d1  # False
'a' in d1  # True

# Swig-generated module
import dictlite_swig as dls
d2 = dls.Dictlite()
len(d2)  # 0
# Other methods do not work; Swig interface incomplete
```


Copyright (c) 2013 Aubrey Barnard.  This is free software.  See
LICENSE.txt for details.
