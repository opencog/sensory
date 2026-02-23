
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY (sensory-sockets SHARED
	UnixSocketNode.cc
	TcpSocketNode.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-sockets sensory_atom_types)

TARGET_LINK_LIBRARIES(sensory-sockets
	sensory
	sensory-types
	${ATOMSPACE_LIBRARIES}
	${COGUTIL_LIBRARY}
)

ADD_GUILE_EXTENSION(SCM_CONFIG sensory-sockets "opencog-ext-path-sensory-sockets")

INSTALL (TARGETS sensory-sockets EXPORT AtomSpaceTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/opencog"
)

INSTALL (FILES
	UnixSocketNode.h
	TcpSocketNode.h
	DESTINATION "include/opencog/atoms/sensory"
)
