
# --------------------------------------------------

OPENCOG_GEN_CXX_ATOMTYPES(sensory_types.script
	atom_types.h
	atom_types.definitions
	atom_types.inheritance)

OPENCOG_GEN_SCM_ATOMTYPES(sensory_types.script sensory_types.scm)
OPENCOG_GEN_PYTHON_ATOMTYPES(sensory_types.script sensory_types.pyx)

ADD_CUSTOM_TARGET(sensory_atom_types
	DEPENDS atom_types.h sensory_types.scm sensory_types.pyx)

# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

ADD_LIBRARY(sensory-types SHARED
	sensory_types_init.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-types sensory_atom_types)

TARGET_LINK_LIBRARIES(sensory-types
	${ATOMSPACE_atomtypes_LIBRARY}
)

INSTALL (TARGETS sensory-types
	EXPORT SensoryTargets
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/opencog"
)

INSTALL (FILES
	${CMAKE_CURRENT_BINARY_DIR}/atom_types.h
	DESTINATION "include/opencog/sensory/types"
)

# Install the auto-generated atom types as well
ADD_GUILE_MODULE (FILES
	${CMAKE_CURRENT_BINARY_DIR}/sensory_types.scm
	MODULE_DESTINATION "${GUILE_SITE_DIR}/opencog/sensory/types"
	DEPENDS sensory_atom_types
)

add_guile_extension(SCM_CONFIG sensory-types "opencog-ext-path-sensory-types")
