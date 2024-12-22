
# This is the name of the constructor in the libsensory-types.so
# shared library. If we don't call it, the sensory Atom types
# don't get initialized correctly for python.
cdef extern void sensory_types_init()
