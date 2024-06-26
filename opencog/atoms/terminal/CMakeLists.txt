
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY (sensory-terminal SHARED
	TerminalStream.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-terminal sensory_atom_types)

TARGET_LINK_LIBRARIES(sensory-terminal
	sensory
	sensory-types
	${ATOMSPACE_LIBRARIES}
	${COGUTIL_LIBRARY}
)

INSTALL (TARGETS sensory-terminal EXPORT AtomSpaceTargets
	DESTINATION "lib${LIB_DIR_SUFFIX}/opencog"
)

INSTALL (FILES
	TerminalStream.h
	DESTINATION "include/opencog/atoms/sensory"
)
