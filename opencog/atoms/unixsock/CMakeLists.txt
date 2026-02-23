
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY (sensory-unixsock SHARED
	UnixSocketNode.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-unixsock sensory_atom_types)

TARGET_LINK_LIBRARIES(sensory-unixsock
	sensory
	sensory-types
	${ATOMSPACE_LIBRARIES}
	${COGUTIL_LIBRARY}
)

ADD_GUILE_EXTENSION(SCM_CONFIG sensory-unixsock "opencog-ext-path-sensory-unixsock")

INSTALL (TARGETS sensory-unixsock EXPORT AtomSpaceTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/opencog"
)

INSTALL (FILES
	UnixSocketNode.h
	DESTINATION "include/opencog/atoms/sensory"
)
