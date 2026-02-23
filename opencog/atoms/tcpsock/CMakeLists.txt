
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY (sensory-tcpsock SHARED
	TcpSocketNode.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-tcpsock sensory_atom_types)

TARGET_LINK_LIBRARIES(sensory-tcpsock
	sensory
	sensory-types
	${ATOMSPACE_LIBRARIES}
	${COGUTIL_LIBRARY}
)

ADD_GUILE_EXTENSION(SCM_CONFIG sensory-tcpsock "opencog-ext-path-sensory-tcpsock")

INSTALL (TARGETS sensory-tcpsock EXPORT AtomSpaceTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/opencog"
)

INSTALL (FILES
	TcpSocketNode.h
	DESTINATION "include/opencog/atoms/sensory"
)
