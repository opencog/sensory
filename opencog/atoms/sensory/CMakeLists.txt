
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY (sensory SHARED
	ReadStream.cc
	SensoryNode.cc
	StreamNode.cc
	TextStream.cc
	TextStreamNode.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory sensory_atom_types)

TARGET_LINK_LIBRARIES(sensory
	sensory-types
	${ATOMSPACE_LIBRARIES}
	${COGUTIL_LIBRARY}
)

INSTALL (TARGETS sensory EXPORT SensoryTargets
	DESTINATION "lib${LIB_DIR_SUFFIX}/opencog"
)

INSTALL (FILES
	ReadStream.h
	SensoryNode.h
	StreamNode.h
	TextStream.h
	TextStreamNode.h
	DESTINATION "include/opencog/atoms/sensory"
)
