
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY (sensory-filedir SHARED
	TextFileStream.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-filedir sensory_atom_types)

TARGET_LINK_LIBRARIES(sensory-filedir
	sensory
	sensory-types
	${ATOMSPACE_LIBRARIES}
	${COGUTIL_LIBRARY}
)

INSTALL (TARGETS sensory-filedir EXPORT AtomSpaceTargets
	DESTINATION "lib${LIB_DIR_SUFFIX}/opencog"
)

INSTALL (FILES
	TextFileStream.h
	DESTINATION "include/opencog/atoms/sensory"
)
