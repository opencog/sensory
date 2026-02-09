
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY (sensory-ollama SHARED
	OllamaNode.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-ollama sensory_atom_types)

TARGET_INCLUDE_DIRECTORIES(sensory-ollama PRIVATE ${HTTPLIB_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(sensory-ollama
	sensory
	sensory-types
	${HTTPLIB_LIBRARIES}
	${ATOMSPACE_LIBRARIES}
	${COGUTIL_LIBRARY}
)

ADD_GUILE_EXTENSION(SCM_CONFIG sensory-ollama "opencog-ext-path-sensory-ollama")

INSTALL (TARGETS sensory-ollama EXPORT AtomSpaceTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/opencog"
)

INSTALL (FILES
	OllamaNode.h
	DESTINATION "include/opencog/atoms/sensory"
)
