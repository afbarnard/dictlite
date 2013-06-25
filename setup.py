# Copyright (c) 2013 Aubrey Barnard.  This is free software.  See
# LICENSE.txt for details.

from distutils.core import setup, Extension
setup(name='dictlite',
      version='0.1',
      ext_modules=[Extension(
            name='dictlite',
            sources=['dictlite_module.c', 'dictlite.c'],
            depends=['dictlite.h'],
            )],
      )
