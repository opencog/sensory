
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

ADD_LIBRARY (sensory-irc SHARED
	IRC.cc
	IRChatNode.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-irc sensory_atom_types)

TARGET_LINK_LIBRARIES(sensory-irc
	sensory
	sensory-types
	${ATOMSPACE_LIBRARIES}
	${COGUTIL_LIBRARY}
)

ADD_GUILE_EXTENSION(SCM_CONFIG sensory-irc "opencog-ext-path-sensory-irc")

INSTALL (TARGETS sensory-irc EXPORT AtomSpaceTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/opencog"
)

INSTALL (FILES
	IRChatNode.h
	DESTINATION "include/opencog/atoms/sensory"
)
