
# --------------------------------------------------

OPENCOG_GEN_CXX_ATOMTYPES(sensory_v0_types.script
	sensory_v0_types.h
	sensory_v0_types.definitions
	sensory_v0_types.inheritance)

OPENCOG_GEN_SCM_ATOMTYPES(sensory_v0_types.script sensory_v0_types.scm)
OPENCOG_GEN_PYTHON_ATOMTYPES(sensory_v0_types.script sensory_v0_types.pyx)

ADD_CUSTOM_TARGET(sensory_v0_atom_types
	DEPENDS sensory_v0_types.h sensory_v0_types.scm sensory_v0_types.pyx)

# The sensory_v0_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

ADD_LIBRARY(sensory-v0-types SHARED
	sensory_v0_types_init.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(sensory-v0-types sensory_v0_atom_types)

TARGET_LINK_LIBRARIES(sensory-v0-types
	${ATOMSPACE_atomtypes_LIBRARY}
)

INSTALL (TARGETS sensory-v0-types
	EXPORT SensoryV0Targets
	LIBRARY DESTINATION "lib${LIB_DIR_SUFFIX}/opencog"
)

INSTALL (FILES
	${CMAKE_CURRENT_BINARY_DIR}/sensory_v0_types.h
	DESTINATION "include/opencog/sensory-v0/types"
)

add_guile_extension(SCM_CONFIG sensory-v0-types "opencog-ext-path-sensory-v0-types")
