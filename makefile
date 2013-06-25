# Copyright (c) 2013 Aubrey Barnard.  This is free software.  See
# LICENSE.txt for details.

# Commands to build C modules for Python

# Commands that do not produce files
.PHONY: all clean

all: main dictlite.so dictlite_swig.py _dictlite_swig.so

# Example program
main: dictlite.h dictlite.c main.c
	gcc -Wall -o $@ $^

# Build hand-wrapped module
dictlite.so: dictlite.h dictlite.c dictlite_module.c
	python setup.py build
	cp build/lib.*/dictlite.so $@  # Could symbolic link this instead

# Build Swig module
dictlite_swig_wrap.c dictlite_swig.py: dictlite_swig.i dictlite.h dictlite.c
	swig -Wall -python $<

dictlite.o: dictlite.c dictlite.h
	gcc -Wall -fPIC -c $<

dictlite_swig_wrap.o: dictlite_swig_wrap.c
	gcc -fPIC -I /usr/include/python2.7 -c $<

_dictlite_swig.so: dictlite.o dictlite_swig_wrap.o
	gcc -shared -o $@ $^

# Clean
clean:
	@rm -f *.so *.o *.pyc dictlite_swig_wrap.c dictlite_swig.py main
	@rm -Rf build
