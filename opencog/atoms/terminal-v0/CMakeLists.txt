
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY (sensory-v0-terminal SHARED
	TerminalStream.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-v0-terminal sensory_v0_atom_types)

TARGET_LINK_LIBRARIES(sensory-v0-terminal
	sensory-v0
	sensory-v0-types
	${ATOMSPACE_LIBRARIES}
	${COGUTIL_LIBRARY}
)

INSTALL (TARGETS sensory-v0-terminal EXPORT AtomSpaceTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/opencog"
)

INSTALL (FILES
	TerminalStream.h
	DESTINATION "include/opencog/atoms/sensory-v0"
)
