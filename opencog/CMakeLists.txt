ADD_SUBDIRECTORY (atoms)

# Python bindings
IF (HAVE_CYTHON)
   ADD_SUBDIRECTORY (cython)
ENDIF (HAVE_CYTHON)
